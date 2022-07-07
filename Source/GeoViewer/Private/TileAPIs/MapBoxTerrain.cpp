#include "TileAPIs/MapBoxTerrain.h"

#include "GDALWarp.h"
#include "GeoViewerSettings.h"
#include "TileDownloader.h"

FMapBoxTerrain::FMapBoxTerrain(TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
                               AWorldReferenceSystem* ReferencingSystem) : FWebMapTileAPI(InEdModeConfig, ReferencingSystem)
{
	UGeoViewerSettings* Settings = GetMutableDefault<UGeoViewerSettings>();

	if (Settings)
	{
		APIKey = Settings->MapboxAPIKey;
	}

	TileResolution = 256;
	ZoomLevel = 14;
}

FMapBoxTerrain::~FMapBoxTerrain()
{
	GDALDriver* Driver = (GDALDriver*)GDALGetDriverByName("GTiff");

	for (FString DatasetPath : GTiffPaths)
	{
		Driver->Delete(TCHAR_TO_UTF8(*DatasetPath));
	}
}

void FMapBoxTerrain::LoadTile(const FProjectedBounds InTileBounds)
{
	TileBounds = InTileBounds;

	if (TileReferenceSystem)
	{
		// Find the bounds in XY coordinates used by Mapbox
		const FGeoBounds GeoBounds = InTileBounds.ConvertToGeoBounds(TileReferenceSystem);
		FVector2D TopLeft = GetSlippyMapCoordinates(GeoBounds.BottomRight);
		FVector2D BottomRight = GetSlippyMapCoordinates(GeoBounds.TopLeft);

		// Make sure both corners are the correct way round
		if (TopLeft.X > BottomRight.X)
		{
			float Temp = TopLeft.X;
			TopLeft.X = BottomRight.X;
			BottomRight.X = Temp;
		}

		if (TopLeft.Y > BottomRight.Y)
		{
			float Temp = TopLeft.Y;
			TopLeft.Y = BottomRight.Y;
			BottomRight.Y = Temp;
		}
		
		FVector2D CurrentPosition = TopLeft;

		while (CurrentPosition.Y <= BottomRight.Y)
		{
			while (CurrentPosition.X <= BottomRight.X)
			{
				// Get URL and filename
				const FString URL = GetTileURL(CurrentPosition);
				const FString FileName = GetFileName(CurrentPosition);

				const FString CacheFolder = GetCacheFolderPath();
				const FString FilePath = CacheFolder + FileName + ".tif";

				// Check if file is cached
				if (FPaths::FileExists(FilePath))
				{
					GDALDatasetRef Dataset(
						(GDALDataset*)GDALOpen(TCHAR_TO_UTF8(*(FilePath)), GA_ReadOnly));
					DatasetsToMerge.Add(ConvertFromRGB(Dataset));
					GDALClose(Dataset.Release());
				} else
				{
					// Setup tile downloader
					const TSharedRef<FTileDownloader> Segment = MakeShared<FTileDownloader>();
					Segment->OnDownloaded.BindRaw(this, &FMapBoxTerrain::OnSegmentCompleted);
					SegmentsDownloaders.Add(Segment);
				
					// Update the dataset with new bounds
					const FVector ProjectedPosition = GetProjectedCoordinate(CurrentPosition);

					FGeographicCoordinates GeoCenter;
					FVector SegmentSize;
					const FVector2D PixelSize =
						GetProjectedPixelSize(ProjectedPosition, GeoCenter, SegmentSize);
					
					
					Segment->SetMetaData(ProjectedPosition, PixelSize, 3857);
					Segment->BeginDownload(URL, FileName);
				}
				
				CurrentPosition.X++;
			}
			CurrentPosition.X = TopLeft.X;
			CurrentPosition.Y++;
		}
		
		SegmentNum = (FMath::Abs(TopLeft.X - BottomRight.X) + 1) * (FMath::Abs(TopLeft.Y - BottomRight.Y) + 1);

		CheckComplete();
	}
}

void FMapBoxTerrain::OnSegmentCompleted(const FTileDownloader* TileDownloader)
{
	if (TileDownloader->FinalDataset)
	{
		GDALDatasetRef MapBoxDataset(TileDownloader->FinalDataset);

		GDALDataset* DatasetToMerge = ConvertFromRGB(MapBoxDataset);
		DatasetsToMerge.Add(DatasetToMerge);

		GDALClose(MapBoxDataset.Release());
		
		CheckComplete();
	}
	else
	{
		TriggerOnCompleted(nullptr);
	}
}

FString FMapBoxTerrain::GetTileURL(const FVector2D Coordinates) const
{
	return
		"https://api.mapbox.com/v4/mapbox.terrain-rgb/"
		+ FString::FromInt(ZoomLevel)
		+ "/"
		+ FString::FromInt(Coordinates.X)
		+ "/"
		+ FString::FromInt(Coordinates.Y)
		+ "@2.png?access_token="
		+ APIKey;
}

FString FMapBoxTerrain::GetFileName(const FVector2D Coordinates)
{
	return "MapboxTerrain" + FString::FromInt(Coordinates.X) + "," + FString::FromInt(Coordinates.Y);
}

FGeographicCoordinates FMapBoxTerrain::GetGeographicCoordinates(const FVector2D Coordinates) const
{
	// https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Implementations
	const float TileNum = FMath::Pow(2.f, ZoomLevel);
	FGeographicCoordinates Result;
	Result.Longitude = Coordinates.X / TileNum * 360 - 180;
	const float LatitudeRad = FMath::Atan(FMath::Sinh(PI * (1 - 2 * Coordinates.Y / TileNum)));
	Result.Latitude = FMath::RadiansToDegrees(LatitudeRad);

	return Result;
}

FVector FMapBoxTerrain::GetProjectedCoordinate(const FVector2D Coordinates) const
{
	const FGeographicCoordinates GeoCoord = GetGeographicCoordinates(Coordinates);

	FVector Result;
	TileReferenceSystem->GeographicToProjectedWithEPSG(GeoCoord, Result, EPSG);
	return Result;
}

FVector2D FMapBoxTerrain::GetSlippyMapCoordinates(const FGeographicCoordinates Coordinates) const
{
	// https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Implementations
	const float TileNum = FMath::Pow(2.f, ZoomLevel);
	const float LatitudeRad = FMath::DegreesToRadians(Coordinates.Latitude);

	// https://mathworld.wolfram.com/InverseHyperbolicSine.html
	auto ASinH = [](const float x){ return FMath::Loge(x + FMath::Sqrt(1 + FMath::Pow(x, 2))); };
	
	const float X = TileNum * ((Coordinates.Longitude + 180) / 360);
	const float Y = (1.0 - ASinH(FMath::Tan(LatitudeRad)) / PI) / 2.0 * TileNum;

	FVector2D Result;
	Result.X = FMath::Floor(X);
	Result.Y = FMath::Floor(Y);
	
	return Result;
}

GDALDataset* FMapBoxTerrain::ConvertFromRGB(GDALDatasetRef& MapboxDataset)
{
	// Read height data from dataset
	TArray<uint8> HeightDataInt8;
	FGDALWarp::GetRawImage(MapboxDataset, HeightDataInt8);

	const int XSize = MapboxDataset->GetRasterXSize();
	const int ChannelNum = MapboxDataset->GetRasterCount();
		
	TArray<float> HeightDataFloat;
	HeightDataFloat.Init(0, HeightDataInt8.Num() / ChannelNum);
	for (int i = 0; i < HeightDataInt8.Num(); i += ChannelNum)
	{
		// Convert from 3 channels down to one
		// https://docs.mapbox.com/data/tilesets/reference/mapbox-terrain-rgb-v1/#elevation-data
		const uint8 R = HeightDataInt8[i];
		const uint8 G = HeightDataInt8[i + 1];
		const uint8 B = HeightDataInt8[i + 2];
		HeightDataFloat[i / ChannelNum] = -10000 + ((R * 256 * 256 + (G * 256) + B) * 0.1);
	}

	// Save converted height data to a new dataset
	FString FileName;
	GDALDataset* Result =
		FGDALWarp::CreateGTiffDataset(HeightDataFloat, XSize, XSize, FileName, ERGBFormat::Gray).Release();

	GTiffPaths.Add(FileName);
	
	// Copy GeoTransform to new dataset 
	GDALSetProjection(Result, MapboxDataset->GetProjectionRef());
	double GeoTransform[6];
	MapboxDataset->GetGeoTransform(GeoTransform);
	Result->SetGeoTransform(GeoTransform);
	
	return Result;
}

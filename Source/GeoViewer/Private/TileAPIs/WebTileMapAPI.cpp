#include "TileAPIs/WebTileMapAPI.h"
#include "GDALWarp.h"

FWebMapTileAPI::FWebMapTileAPI(const TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
                               AWorldReferenceSystem* ReferencingSystem) :
	FGeoTileAPI(InEdModeConfig, ReferencingSystem), ZoomLevel(0), TileResolution(0)
{
	SegmentNum = -1;
}

void FWebMapTileAPI::LoadTile(const FProjectedBounds InTileBounds)
{
	TileBounds = InTileBounds;
	
	if (TileReferenceSystem)
	{
		// Get the bounds in projected coordinates used by the data source
		auto [TopLeft, BottomRight] = GetProjectedBounds();
		
		// Download all segments needed till the 'CurrentPosition' is beyond the bottom corner
		FVector CurrentPosition = TopLeft;
		
		FVector2D PositionIndex = FVector2D(0, 0); // Position of a segment in relation to the other segments.
		while (CurrentPosition.Y > BottomRight.Y)
		{
			PositionIndex.X = 0;
			FVector ProjectedSegmentSize;
			while (CurrentPosition.X < BottomRight.X)
			{
				FGeographicCoordinates SegmentCenterGeo;
				const FVector2D PixelSize =
					GetProjectedPixelSize(CurrentPosition, SegmentCenterGeo, ProjectedSegmentSize);

				//Get URL and filename
				const FString URL = GetTileURL(SegmentCenterGeo);
				const FString FileName = GetFileName(SegmentCenterGeo);

				const FString CacheFolder = GetCacheFolderPath();
				const FString FilePath = CacheFolder + FileName + ".tif";

				// Check if file is cached
				if (FPaths::FileExists(FilePath))
				{
					GDALDataset* Dataset = (GDALDataset*)GDALOpen(TCHAR_TO_UTF8(*(FilePath)), GA_ReadOnly);
					DatasetsToMerge.Add(Dataset);
				} else
				{
					// Setup tile downloader
					const TSharedRef<FTileDownloader> Segment = MakeShared<FTileDownloader>();
					Segment->OnDownloaded.BindRaw(this, &FWebMapTileAPI::OnSegmentCompleted);
					SegmentsDownloaders.Add(Segment);
				
					// Update the dataset with new bounds
					Segment->SetMetaData(CurrentPosition, PixelSize, 3857);
					Segment->BeginDownload(URL, FileName);
				}
				
				
				// Move position along
				CurrentPosition.X += ProjectedSegmentSize.X;
				PositionIndex.X++;
			}
			
			// After each row reset X and increment Y
			CurrentPosition.X = TopLeft.X;
			CurrentPosition.Y -= ProjectedSegmentSize.Y;
			
			PositionIndex.Y++;
		}

		SegmentNum = PositionIndex.X * PositionIndex.Y;
	}

	CheckComplete();
}

void FWebMapTileAPI::CheckComplete()
{
	// See if all tiles have downloaded
	if (SegmentNum > 0 && DatasetsToMerge.Num() == SegmentNum)
	{
		// As the download has complete none of the downloaders are needed
		SegmentsDownloaders.Empty();
		
		// At this point all segments have been downloaded so the final dataset can be created.
		GDALDataset* MergedDataset = MergeDatasets();
		if (!MergedDataset)
		{
			TriggerOnCompleted(nullptr);
		}
		
		const int MergedDatasetIdx = CachedDatasets.Add(GDALDatasetRef(MergedDataset));
		
		GDALDataset* WarpedDataset = WarpDataset(MergedDatasetIdx);
		CachedDatasets.Add(GDALDatasetRef(WarpedDataset));

		// Crop the dataset down the the correct bounds.
		FString CroppedDatasetPath;
		GDALDataset* CroppedDataset = FGDALWarp::CropDataset(
			WarpedDataset,
			TileBounds.TopLeft,
			TileBounds.BottomRight,
			CroppedDatasetPath
			).Release();
		CachedDatasetPaths.Add(CroppedDatasetPath);
		
		TriggerOnCompleted(CroppedDataset);
	}
}

void FWebMapTileAPI::OnSegmentCompleted(const FTileDownloader* TileDownloader)
{
	if (TileDownloader->FinalDataset)
	{
		DatasetsToMerge.Add(TileDownloader->FinalDataset);
		CheckComplete();
	}
	else
	{
		TriggerOnCompleted(nullptr);
	}
}

float FWebMapTileAPI::CalculateTileSize(double Latitude) const
{
	// https://docs.microsoft.com/en-us/bingmaps/articles/understanding-scale-and-resolution
	// Meters/Pixel
	const float PixelSize = (FMath::Cos(Latitude * PI / 180) * 156543.03392) / FMath::Pow(2.0f, ZoomLevel);
	
	return PixelSize * TileResolution;
}

FVector2D FWebMapTileAPI::GetProjectedPixelSize(const FVector TopCorner, FGeographicCoordinates& SegmentCenter, FVector& SegmentSize) const
{
	// Get the corner position for the next segment in geographic coordinates
	FGeographicCoordinates SegmentTopCornerGeo;
	TileReferenceSystem->ProjectedToGeographicWithEPSG(TopCorner, SegmentTopCornerGeo, EPSG);
	
	// Calculate side length of the segment
	const float HalfTileSize = CalculateTileSize(SegmentTopCornerGeo.Latitude) / 2;

	// Get the center position of the tile in geographic coordinates
	AGeoViewerReferenceSystem::GetGeographicalCoordinatesAtOffset(
		SegmentTopCornerGeo,
		FVector2D(HalfTileSize, -HalfTileSize),
		SegmentCenter
		);

	// Convert geographic top corner to projected coordinates
	FVector SegmentCenterProj;
	TileReferenceSystem->GeographicToProjectedWithEPSG(SegmentCenter, SegmentCenterProj, EPSG);

	// Calculate segment size in the projected CRS units
	SegmentSize.X = FMath::Abs(TopCorner.X - SegmentCenterProj.X) * 2;
	SegmentSize.Y = FMath::Abs(TopCorner.Y - SegmentCenterProj.Y) * 2;

	FVector2D PixelSize;
	PixelSize.X = SegmentSize.X / TileResolution;
	PixelSize.Y = SegmentSize.Y / TileResolution;

	return PixelSize;
}

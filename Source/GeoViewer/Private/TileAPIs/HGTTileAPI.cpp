#include "TileAPIs/HGTTileAPI.h"

#include "GDALWarp.h"
#include "Interfaces/IPluginManager.h"

FHGTTileAPI::FHGTTileAPI(
	const TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
	AWorldReferenceSystem* ReferencingSystem) :
	FGeoTileAPI(InEdModeConfig, ReferencingSystem)
{
	EPSG = 4326;
}

void FHGTTileAPI::LoadTile(FProjectedBounds InTileBounds)
{
	TileBounds = InTileBounds;

	const FProjectedBounds ProjectedBounds = GetProjectedBounds();
	FGeoBounds Bounds;
	Bounds.TopLeft = ProjectedBounds.TopLeft;
	Bounds.BottomRight = ProjectedBounds.BottomRight;
	
	FGeographicCoordinates CurrentPosition = Bounds.TopLeft;
	CurrentPosition.Longitude = Bounds.BottomRight.Longitude;

	// Load all terrain files that are needed to cover the bounds
	while (CurrentPosition.Longitude < Bounds.TopLeft.Longitude)
	{
		while (CurrentPosition.Latitude < Bounds.BottomRight.Latitude)
		{
			OpenDataset(CurrentPosition);

			// Increase Longitude by 1 degree
			CurrentPosition.Latitude = FMath::Floor(CurrentPosition.Latitude) + 1;
		}

		// Decrease the latitude by 1 degree
		CurrentPosition.Longitude = FMath::Floor(CurrentPosition.Longitude) + 1;

		// Set the X coordinate back to the left
		CurrentPosition.Latitude = Bounds.TopLeft.Latitude;
	}
	
	GDALDataset* MergedDataset = MergeDatasets();
	const int MergedDatasetIdx = CachedDatasets.Add(GDALDatasetRef(MergedDataset));
	GDALDataset* WarpedDataset = WarpDataset(MergedDatasetIdx);
	const int WarpedDatasetIdx = CachedDatasets.Add(GDALDatasetRef(WarpedDataset));
	
	FString CroppedDatasetPath;
	GDALDataset* CroppedDataset = FGDALWarp::CropDataset(
		CachedDatasets[WarpedDatasetIdx].Get(),
		TileBounds.TopLeft,
		TileBounds.BottomRight,
		CroppedDatasetPath
		).Release();
	CachedDatasetPaths.Add(CroppedDatasetPath);
	
	TriggerOnCompleted(CroppedDataset);
}

int FHGTTileAPI::OpenDataset(const FGeographicCoordinates PositionWithinTile)
{
	const FString TerrainFolder = GetTerrainFolder();
	const FString TileFileName = TerrainFolder + GetFileName(PositionWithinTile);
	if (FPaths::FileExists(TileFileName))
	{
		GDALDataset* Dataset = (GDALDataset*)GDALOpen(TCHAR_TO_UTF8(*TileFileName), GA_ReadOnly);
		return DatasetsToMerge.Add(Dataset);
	}

	return -1;
}

FString FHGTTileAPI::GetFileName(const FGeographicCoordinates Coordinates) const
{
	const int LatitudeInt = FMath::Floor(Coordinates.Latitude);
	const int LongitudeInt = FMath::Floor(Coordinates.Longitude);

	FString FileName = "";

	if (LongitudeInt >= 0)
	{
		FileName += "N";
	} else
	{
		FileName += "S";
	}

	FileName += ConvertIntToString(FMath::Abs(LongitudeInt), 2);

	if (LatitudeInt > 0)
	{
		FileName += "E";
	} else
	{
		FileName += "W";
	}

	FileName += ConvertIntToString(FMath::Abs(LatitudeInt), 3);
	FileName += ".hgt";

	return FileName;
}

FString FHGTTileAPI::GetTerrainFolder()
{
	const TSharedPtr<IPlugin> PluginManager = IPluginManager::Get().FindPlugin(TEXT("GeoViewer"));
	return PluginManager->GetBaseDir() + TEXT("/Resources/Terrain/HGT/");
}

FString FHGTTileAPI::ConvertIntToString(const int Number, int NumOfDigits)
{
	FString NumberString = FString::FromInt(Number);

	for (int i = NumberString.Len(); i < NumOfDigits; i++)
	{
		NumberString = "0" + NumberString;
	}

	return NumberString;
}

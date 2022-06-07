﻿#include "TileAPIS/GeoTileAPI.h"

#include "GDALWarp.h"
#include "Interfaces/IPluginManager.h"

FGeoTileAPI::FGeoTileAPI(
	TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
	AWorldReferenceSystem* ReferencingSystem
	)
{
	EdModeConfigPtr = InEdModeConfig;
	TileReferenceSystem = ReferencingSystem;
}

FGeoTileAPI::~FGeoTileAPI()
{
	EmptyDatasetsToMerge();
	CachedDatasets.Empty();

	// Delete any vrt datasets stored in memory
	GDALDriver* Driver = (GDALDriver*)GDALGetDriverByName("VRT");

	for (FString DatasetPath : CachedDatasetPaths)
	{
		Driver->Delete(TCHAR_TO_UTF8(*DatasetPath));
	}
}

FString FGeoTileAPI::GetCacheFolderPath()
{
	const TSharedPtr<IPlugin> PluginManager = IPluginManager::Get().FindPlugin(TEXT("GeoViewer"));
	return PluginManager->GetBaseDir() + TEXT("/Resources/CachedTiles/");
}

void FGeoTileAPI::TriggerOnCompleted(GDALDataset* Dataset) const
{
	OnComplete.ExecuteIfBound(Dataset);
}

GDALDataset* FGeoTileAPI::WarpDataset(int CachedDatasetIdx)
{
	const FString CurrentCRS = AGeoViewerReferenceSystem::EPSGToString(EPSG);
	const FString FinalCRS = TileReferenceSystem->ProjectedCRS;
	GDALDatasetRef WarpedDataset =
		FGDALWarp::WarpDataset(CachedDatasets[CachedDatasetIdx], CurrentCRS, FinalCRS);
	return WarpedDataset.Release();
}

GDALDataset* FGeoTileAPI::MergeDatasets()
{
	std::vector<GDALDataset*> DatasetsVector;

	for (GDALDataset* Dataset : DatasetsToMerge)
	{
		DatasetsVector.push_back(Dataset);
	}

	int OutputError = FALSE;

	GDALAllRegister();
	
	GDALDataset* MergedDataset = (GDALDataset*)GDALBuildVRT(
			"",
			DatasetsVector.size(),
			(GDALDatasetH*)DatasetsVector.data(),
			nullptr,
			nullptr,
			&OutputError
			);

	EmptyDatasetsToMerge();
	
	if (MergedDataset)
	{
		return MergedDataset;
	}

	return nullptr;
}

void FGeoTileAPI::EmptyDatasetsToMerge()
{
	for (GDALDataset* Dataset : DatasetsToMerge)
	{
		GDALClose(Dataset);
		Dataset = nullptr;
	}

	DatasetsToMerge.Empty();
}

FProjectedBounds FGeoTileAPI::GetProjectedBounds() const
{
	// Calculate all four corners of the tile.
	const double XMin = TileBounds.TopLeft.X;
	const double XMax = TileBounds.BottomRight.X;
	const double YMin = TileBounds.BottomRight.Y;
	const double YMax = TileBounds.TopLeft.Y;

	TArray<FVector> Corners; // UE world projected CRS
	Corners.Add(FVector(XMin, YMax, 0));
	Corners.Add(FVector(XMax, YMax, 0));
	Corners.Add(FVector(XMin, YMin, 0));
	Corners.Add(FVector(XMax, YMin, 0));

	// Convert corners to geographical coordinates then back into the projected CRS
	// of the source data.
	TArray<FGeographicCoordinates> GeoCorners;
	for (FVector Corner : Corners)
	{
		FGeographicCoordinates NewGeoCoord;
		TileReferenceSystem->ProjectedToGeographic(Corner, NewGeoCoord);
		GeoCorners.Add(NewGeoCoord);
	}

	// Coordinates in the CRS used by the source data
	TArray<FVector> CornersProj;
	
	for (FGeographicCoordinates GeoCorner : GeoCorners)
	{
		FVector NewProjCoord;
		TileReferenceSystem->GeographicToProjectedWithEPSG(GeoCorner, NewProjCoord, EPSG);
		CornersProj.Add(NewProjCoord);
	}

	// Find the coordinates need to form a square tile in the projected CRS used by
	// the source data. This should prevent missing side sections of the tile when
	// cropped back to the original bounds in a different projection.
	double ProjXMax = CornersProj[0].X;
	double ProjXMin = CornersProj[0].X;
	double ProjYMax = CornersProj[0].Y;
	double ProjYMin = CornersProj[0].Y;

	for (int i = 1; i < CornersProj.Num(); i++)
	{
		if (CornersProj[i].X > ProjXMax)
		{
			ProjXMax = CornersProj[i].X;
		}
		else if (CornersProj[i].X < ProjXMin)
		{
			ProjXMin = CornersProj[i].X;
		}

		if (CornersProj[i].Y > ProjYMax)
		{
			ProjYMax = CornersProj[i].Y;
		}
		else if (CornersProj[i].Y < ProjYMin)
		{
			ProjYMin = CornersProj[i].Y;
		}
	}
	
	FProjectedBounds IncreasedBounds;
	IncreasedBounds.TopLeft = FVector(ProjXMin, ProjYMax, 0);
	IncreasedBounds.BottomRight = FVector(ProjXMax, ProjYMin, 0);

	return IncreasedBounds;
}

FGeoBounds FGeoTileAPI::GetGeographicBounds() const
{
	FGeoBounds GeoBounds;
	TileReferenceSystem->ProjectedToGeographic(TileBounds.TopLeft, GeoBounds.TopLeft);
	TileReferenceSystem->ProjectedToGeographic(TileBounds.BottomRight, GeoBounds.BottomRight);

	return GeoBounds;
}

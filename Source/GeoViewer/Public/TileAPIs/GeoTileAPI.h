#pragma once
#include "GeographicCoordinates.h"
#include "GDALSmartPointers.h"
#include "ReferenceSystems/WorldReferenceSystem.h"

/** Holds the corner coordinates in lon, lat for a tile */
struct FGeoBounds
{
	FGeographicCoordinates TopLeft;
	FGeographicCoordinates BottomRight;
};

/** Holds the corner coordinates in a projected CRS */
struct FProjectedBounds
{
	FVector TopLeft;
	FVector BottomRight;
};

/**
 * Abstract class used to interface between different map systems
 * and create tiles of the specified bounds in the form of a GDALDataset
 * projected into the correct projection for the current UE world.
 */
class FGeoTileAPI : public TSharedFromThis<FGeoTileAPI>
{
public:
	DECLARE_DELEGATE_OneParam(FOnComplete, GDALDataset*)

	FGeoTileAPI(
		TWeakObjectPtr<class UGeoViewerEdModeConfig> InEdModeConfig,
		AWorldReferenceSystem* ReferencingSystem
		);
	
	virtual ~FGeoTileAPI();

	/** Called when tiles should begin to be loaded. */
	virtual void LoadTile(FProjectedBounds TileBounds) = 0;

	/** Returns the path to the folder containing cached images */
	static FString GetCacheFolderPath();
	
	/** Delegate to functions to be called once complete */
	FOnComplete OnComplete;
protected:
	/** Calls the on complete delegate for when the dataset has been loaded */
	void TriggerOnCompleted(GDALDataset* Dataset) const;

	/**
	 * Warps a dataset to the CRS used by the world.
	 * @param CachedDatasetIdx Index of dataset in 'CachedDatasets' array.
	 */
	GDALDataset* WarpDataset(int CachedDatasetIdx);
	
	/** Merges all datasets into one */
	GDALDataset* MergeDatasets();

	/** Closes all items in the array DatasetsToMerge then empties the array */
	void EmptyDatasetsToMerge();

	/**
	 * Calculates the projected bounds in the CRS of the source data.
	 * As the CRS being used may be rotated in comparison to the UE world,
	 * the coordinates returned will be bigger to ensure there isn't
	 * any data missing from the corners of the requested tile.
	 */
	FProjectedBounds GetProjectedBounds() const;

	/** Returns the tile bounds in geographic coordinates. */
	FGeoBounds GetGeographicBounds() const;
	
	/** Reference system for converting to a different CRS */
	AWorldReferenceSystem* TileReferenceSystem;

	/** Datasets that may be needed later on when converting to a raw image */
	TArray<GDALDatasetRef> CachedDatasets;

	/** Paths of vrt datasets that should be deleted when this object is destroyed */
	TArray<FString> CachedDatasetPaths;
	
	/** All the segments needed to form one big dataset */
	TArray<GDALDataset*> DatasetsToMerge;

	/** Contains API keys and details for the overlay */
	TWeakObjectPtr<UGeoViewerEdModeConfig> EdModeConfigPtr;

	/** Projected bounds of the tile being loaded in the CRS used by the UE world. */
	FProjectedBounds TileBounds;
	
	/** CRS used by dataset */
	uint16 EPSG = 3857;
};

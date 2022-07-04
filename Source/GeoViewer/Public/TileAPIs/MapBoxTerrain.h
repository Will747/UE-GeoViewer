#pragma once
#include "TileDownloader.h"
#include "WebTileMapAPI.h"

/**
 * Class for downloading DEM data from MapBox using the Terrain-RGB tile set.
 */
class FMapBoxTerrain : public FWebMapTileAPI
{
public:
	FMapBoxTerrain(
		TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
		AWorldReferenceSystem* ReferencingSystem
		);

	virtual ~FMapBoxTerrain() override;

	// FGeoTileAPI Interface
	virtual void LoadTile(FProjectedBounds InTileBounds) override;
	// End FGeoTileAPI Interface

protected:
	// FWebMapTileAPI Interface
	/** Not in use as replaced by functions with FVector2D parameter. */
	virtual FString GetTileURL(FGeographicCoordinates Coordinates) const override { return FString(); };
	virtual FString GetFileName(FGeographicCoordinates Coordinates) const override { return FString(); };
	
	virtual void OnSegmentCompleted(const FTileDownloader* TileDownloader) override;
	// End FWebMapTileAPI Interface
private:
	/** Returns URL to tile at specific slippy map coordinates. */ 
	FString GetTileURL(const FVector2D Coordinates) const;

	/** Returns filename for cached tile at specific slippy map coordinates. */
	static FString GetFileName(const FVector2D Coordinates);

	/** Converts slippy map coordinates to geographic. */
	FGeographicCoordinates GetGeographicCoordinates(const FVector2D Coordinates) const;

	/** Converts slippy map coordinate to EPSG:3857 coordinate. */
	FVector GetProjectedCoordinate(const FVector2D Coordinates) const;

	/** Converts geographic coordinates to slippy map coordinates. */
	FVector2D GetSlippyMapCoordinates(const FGeographicCoordinates Coordinates) const;

	/** Transforms an RGB dataset to one with one channel containing height data in meters. */
	GDALDataset* ConvertFromRGB(GDALDatasetRef& MapboxDataset);

	/** Mapbox API key */
	FString APIKey;

	/** Paths of temporary GTiff files that get deleted when this object is deleted. */ 
	TArray<FString> GTiffPaths;
};

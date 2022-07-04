#pragma once
#include "GeoTileAPI.h"
#include "TileDownloader.h"

/**
 * Abstract class for downloading tiles from popular static map APIs
 * such as google and bing that use a very similar format.
 */
class FWebMapTileAPI : public FGeoTileAPI
{
public:
	FWebMapTileAPI(
		TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
		AWorldReferenceSystem* ReferencingSystem
		);
	
	virtual void LoadTile(FProjectedBounds InTileBounds) override;

protected:
	/** 
	 * Generates a unique filename based on config and coordinates.
	 * @param Coordinates The geographical center position of the tile.
	 * @return The filename for the image at specified coordinates.
	 */
	virtual FString GetFileName(FGeographicCoordinates Coordinates) const = 0;
	
	/**
	 * Generates the url for the tile based on config and coordinates.
	*  @param Coordinates The geographical center position of the tile.
	 * @return The url to the image at specified coordinates.
	 */
	virtual FString GetTileURL(FGeographicCoordinates Coordinates) const = 0;

	/**
	 * Once a segment has been downloaded and converted to a Dataset this method is called.
	 * It adds the completed dataset to the array of datasets to merge and checks if all datasets are added.
	 * @param TileDownloader The object calling this function.
	 */
	virtual void OnSegmentCompleted(const FTileDownloader* TileDownloader);

	/**
	 * Calculates the side length for an image based on latitude, zoom level and resolution.
	 * @param Latitude The latitude position of the image.
	 * @return The side length of the image in meters.
	 */
	float CalculateTileSize(double Latitude) const;

	/**
	 * Returns the distance in projected units per pixel.
	 * @param TopCorner The top corner of the segment in projected crs.
	 * @param SegmentCenter The geographic center position of the segment.
	 * @param SegmentSize The size of the segment in projected units.
	 */
	FVector2D GetProjectedPixelSize(FVector TopCorner, FGeographicCoordinates& SegmentCenter, FVector& SegmentSize) const;

	/**
	 * Checks if all segments needed to form the final tile are ready, then merges all
	 * segments into one dataset, warps the dataset to the one used by the world then
	 * executes the onComplete delegate.
	 */
	void CheckComplete();
	
	/** The scale of a segment, where 0 is the entire earth and buildings are at 20 */
	int ZoomLevel;

	/** The side length of one segment in number of pixels */
	int TileResolution;
	
	/** Reference to the object downloading the tile to prevent garbage collection */
	TArray<TSharedRef<FTileDownloader>> SegmentsDownloaders;
	
	/** Required number of segments to form the tile */
	int SegmentNum;
};

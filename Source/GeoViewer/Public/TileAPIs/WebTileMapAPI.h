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
	
	virtual void LoadTile(FGeoBounds TileBounds) override;

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

	/** The scale of a segment, where 0 is the entire earth and buildings are at 20 */
	int ZoomLevel;

	/** The side length of one segment in number of pixels */
	int TileResolution;
	
private:
	/**
	 * Checks if all segments needed to form the final tile are ready, then merges all
	 * segments into one dataset, warps the dataset to the one used by the world then
	 * executes the onComplete delegate.
	 */
	void CheckComplete();
	
	/**
	 * Once a segment has been downloaded and converted to a Dataset this method is called.
	 * It adds the completed dataset to the array of datasets to merge and checks if all datasets are added.
	 * @param TileDownloader The object calling this function.
	 */
	void OnSegmentCompleted(const FTileDownloader* TileDownloader);

	/**
	 * Calculates the side length for an image based on latitude, zoom level and resolution.
	 * @param Latitude The latitude position of the image.
	 * @return The side length of the image in meters.
	 */
	int CalculateTileSize(double Latitude) const;
	
	/** Reference to the object downloading the tile to prevent garbage collection */
	TArray<TSharedRef<FTileDownloader>> SegmentsDownloaders;

	/** Required number of segments to form the tile */
	int SegmentNum;

	/** CRS used by Bing and Google */
	const uint16 EPSG = 3857;
};

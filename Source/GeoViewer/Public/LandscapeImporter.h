#pragma once
#include "GeoViewerEdModeConfig.h"
#include "TileAPIs/GeoTileAPI.h"

/**
 * Imports landscapes from GIS data into the world. In
 * individual tiles.
 */
class FLandscapeImporter
{
public:
	FLandscapeImporter();

	/** Prepares the importer for adding landscapes to the world */
	void Initialize(UWorld* InWorld, UGeoViewerEdModeConfig* InEdModeConfig);
	
	void LoadTile(FGeoBounds TileBounds);

private:
	UWorld* World;
	UGeoViewerEdModeConfig* EdModeConfig;
	TSharedPtr<FGeoTileAPI> TileAPI;
};

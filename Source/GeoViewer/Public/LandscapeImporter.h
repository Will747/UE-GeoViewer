#pragma once
#include "GeoViewerEdModeConfig.h"
#include "Landscape.h"
#include "TileAPIs/GeoTileAPI.h"
#include "TileAPIs/HGTTileAPI.h"

/**
 * Imports landscapes from GIS data into the world. In
 * individual tiles.
 */
class FLandscapeImporter
{
public:
	FLandscapeImporter();

	/** Prepares the importer for adding landscapes to the world. */
	void Initialize(UWorld* InWorld, UGeoViewerEdModeConfig* InEdModeConfig);

	/**
	 * Adds a new landscape tile to the world.
	 * @param LandscapePosition Coordinates near to where the tile will be added.
	 */
	void LoadTile(FVector LandscapePosition);

private:
	/** Returns the total number of quads on one side of a landscape actor. */
	float GetNumOfQuadsOneAxis() const;

	/** Returns the number of vertices on one side of a landscape actor. */
	float GetNumOfVerticesOneAxis() const;

	/** Returns the scale to be used by all landscape actors. */
	FVector GetLandscapeScale() const;
	
	/** Called when the DEM data has been loaded. */
	void OnTileDataLoaded(GDALDataset* Dataset);

	/** Creates landscape streaming proxy actor. */
	void CreateLandscapeProxy(const TArray<uint16>& HeightData) const;

	/** Returns landscape actor in the world or creates a new one. */
	ALandscape* GetLandscapeActor() const;

	/** Loads weight maps for each layer of the landscape in the specified bounds. */
	void ImportWeightMap(TArray<TArray<uint8>>& RawData, FProjectedBounds Bounds) const;

	/** Gets API being used to import landscapes. */
	TSharedRef<FGeoTileAPI> GetTileAPI();
	
	UWorld* World;
	UGeoViewerEdModeConfig* EdModeConfig;
	
	TArray<TArray<uint8>> WeightMaps;

	/** Used to prevent the object being deleted */
	TSharedPtr<FGeoTileAPI> CachedTileAPI;
	
	/** The offset from the origin in quads */
	FIntPoint CurrentSectionOffset;

	/** Height of mount everest in meters */
	const float MountEverestHeight = 8849;
};

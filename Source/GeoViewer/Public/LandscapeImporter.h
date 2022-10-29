#pragma once
#include "GeoViewerEdModeConfig.h"
#include "Landscape.h"
#include "TileAPIs/GeoTileAPI.h"

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
	 * @param bLoadManyTiles If a window should open for selecting the number of tiles to load.
	 */
	void LoadTile(FVector LandscapePosition, bool bLoadManyTiles = false);

private:
	/** Returns the total number of quads on one side of a landscape actor. */
	float GetNumOfQuadsOneAxis() const;

	/** Returns the number of vertices on one side of a landscape actor. */
	float GetNumOfVerticesOneAxis() const;

	/** Returns the scale to be used by all landscape actors. */
	FVector GetLandscapeScale() const;
	
	/** Called when the DEM data has been loaded. */
	void OnTileDataLoaded(GDALDataset* Dataset);

	/**
	 *	Creates landscape streaming proxy actor.
	 *	@param HeightData The height in correct scale for UE.
	 *	@param LandscapePos Position relative to other landscape proxies currently being imported.
	 */
	void CreateLandscapeProxy(const TArray<uint16>& HeightData, const FIntVector2 LandscapePos) const;

	/** Returns landscape actor in the world or creates a new one. */
	ALandscape* GetLandscapeActor() const;

	/** Loads weight maps for each layer of the landscape in the specified bounds. */
	void ImportWeightMap(TArray<TArray<uint8>>& RawData, FProjectedBounds Bounds) const;

	/** Returns the image size for all landscape proxies being loaded. */
	FIntVector2 GetTotalSize() const;

	/** Converts the index of a landscape tile to a position based on Quads. */
	FIntPoint GetSectionOffset(FIntPoint LandscapeIndex) const;

	/** Crops an image down to specified area. */
	template<typename T>
	static void Crop(FIntVector2& OriginalSize, const int TileLength, const FIntVector2 Pos, const TArray<T>& SourceImage,
	TArray<T>& CroppedImage);

	/** Gets API being used to import landscapes. */
	TSharedRef<FGeoTileAPI> GetTileAPI();
	
	UWorld* World;
	UGeoViewerEdModeConfig* EdModeConfig;
	
	TArray<TArray<uint8>> WeightMaps;

	/** Used to prevent the object being deleted until the tile has loaded. */
	TSharedPtr<FGeoTileAPI> CachedTileAPI;
	
	/** The index of the bottom corner landscape tile being loaded. */
	FIntPoint BottomCornerIndex;

	/** The number of landscape tiles being loaded. */
	FVector2D NumOfTiles;
	
	/** Height of mount everest in meters */
	const float MountEverestHeight = 8849;
};

template <typename T>
void FLandscapeImporter::Crop(FIntVector2& OriginalSize, const int TileLength, const FIntVector2 Pos, const TArray<T>& SourceImage,
	TArray<T>& CroppedImage)
{
	const float MinX = TileLength * Pos.X;
	const float MinY = TileLength * Pos.Y;

	CroppedImage.Empty(TileLength * TileLength);
	
	for (int y = 0; y < TileLength; y++)
	{
		for (int x = 0; x < TileLength; x++)
		{
			const int Index = x + MinX + y * OriginalSize.X + MinY * OriginalSize.X;
			CroppedImage.Add(SourceImage[Index]);
		}
	}	
}

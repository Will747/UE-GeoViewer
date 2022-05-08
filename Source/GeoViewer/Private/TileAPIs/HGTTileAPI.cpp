#include "TileAPIs/HGTTileAPI.h"

FHGTTileAPI::FHGTTileAPI(
	const TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
	AWorldReferenceSystem* ReferencingSystem) :
	FGeoTileAPI(InEdModeConfig, ReferencingSystem)
{
}

void FHGTTileAPI::LoadTile(FGeoBounds TileBounds)
{
}

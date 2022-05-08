#include "LandscapeImporter.h"

#include "TileAPIs/HGTTileAPI.h"

FLandscapeImporter::FLandscapeImporter(): World(nullptr), EdModeConfig(nullptr)
{
}

void FLandscapeImporter::Initialize(UWorld* InWorld, UGeoViewerEdModeConfig* InEdModeConfig)
{
	World = InWorld;
	EdModeConfig = InEdModeConfig;

	if (EdModeConfig && World)
	{
		switch (EdModeConfig->LandscapeFormat)
		{
		case ELandscapeFormat::STRM:
		default:
			TileAPI = MakeShared<FHGTTileAPI>(
				InEdModeConfig,
				AWorldReferenceSystem::GetWorldReferenceSystem(World)
				);
		}
	}
}

void FLandscapeImporter::LoadTile(FGeoBounds TileBounds)
{
	if (EdModeConfig && World)
	{
		
		
		
	}
}

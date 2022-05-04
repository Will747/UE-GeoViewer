#include "OverlayTileGenerator.h"
#include "GDALWarp.h"
#include "MapOverlayActor.h"
#include "TileAPIs/BingMapsAPI.h"
#include "TileAPIs/GoogleMapsAPI.h"

FOverlayTileGenerator::FOverlayTileGenerator(): ParentActor(nullptr)
{
}

FOverlayTileGenerator::~FOverlayTileGenerator()
{
}

void FOverlayTileGenerator::GenerateTile(AMapOverlayActor* InParentActor,
                                         TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
                                         AWorldReferenceSystem* ReferencingSystem,
                                         FGeoBounds TileBounds)
{
	ParentActor = InParentActor;
	
	if (InEdModeConfig->OverlaySystem == EOverlayMapSystem::BingMaps)
	{
		TileLoader = MakeShared<FBingMapsAPI>(InEdModeConfig, ReferencingSystem, TileBounds);
	} else
	{
		TileLoader = MakeShared<FGoogleMapsAPI>(InEdModeConfig, ReferencingSystem, TileBounds);	
	}
	
	TileLoader->OnComplete.BindRaw(this, &FOverlayTileGenerator::OnTileFinishedLoading);
	TileLoader->LoadTile();
}

void FOverlayTileGenerator::OnTileFinishedLoading(GDALDataset* Dataset) const
{
	ParentActor->AddOverlayTile(Dataset, Key);
}

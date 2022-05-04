#pragma once

#include "CoreMinimal.h"
#include "GeoViewerEdModeConfig.h"
#include "TileAPIs/BingMapsAPI.h"

class AMapOverlayActor;

/**
 * Loads tiles from the geo tile api and then adds
 * an overlay tile to the parent actor containing the
 * GDALDataset that just got created
 */
class FOverlayTileGenerator : public TSharedFromThis<FOverlayTileGenerator>
{
public:
	FOverlayTileGenerator();
	virtual ~FOverlayTileGenerator();

	/** Begins the process of downloading and creating a tile for a specific area */
	void GenerateTile(
		AMapOverlayActor* InParentActor,
		TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
		AWorldReferenceSystem* ReferencingSystem,
		FGeoBounds TileBounds
		);

	/** Used to identify the tile being loaded */
	FString Key;
	
private:
	/** Called when the tile has finished downloading and can be added to the parent actor */
	void OnTileFinishedLoading(GDALDataset* Dataset) const;
	
	TSharedPtr<FGeoTileAPI> TileLoader;
	AMapOverlayActor* ParentActor;
};

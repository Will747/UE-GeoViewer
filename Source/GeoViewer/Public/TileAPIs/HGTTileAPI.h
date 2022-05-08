#pragma once
#include "GeoTileAPI.h"

/**
 * Class for loading STRM tiles in the HGT format.
 */
class FHGTTileAPI : public FGeoTileAPI
{
public:
	FHGTTileAPI(
		TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
		AWorldReferenceSystem* ReferencingSystem
		);

	virtual void LoadTile(FGeoBounds TileBounds) override;
};

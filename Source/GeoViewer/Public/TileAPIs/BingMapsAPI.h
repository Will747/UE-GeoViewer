#pragma once
#include "WebTileMapAPI.h"

/**
 * Class for downloading tiles from the Bing Maps imagery API.
 */
class FBingMapsAPI : public FWebMapTileAPI
{
public:
	FBingMapsAPI(
		TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
		AWorldReferenceSystem* ReferencingSystem
		);

protected:
	// FWebMapTileAPI Interface
	virtual FString GetFileName(FGeographicCoordinates Coordinates) const override;
	virtual FString GetTileURL(FGeographicCoordinates Coordinates) const override;
	// End FWebMapTileAPI Interface

private:
	/** Converts map type enum to a string ready for the API request */
	static FString GetMapTypeStr(EBingMapType Type);

	FString APIKey;
	FString MapType;
};

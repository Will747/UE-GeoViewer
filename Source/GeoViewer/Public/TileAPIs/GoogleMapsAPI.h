#pragma once
#include "WebTileMapAPI.h"

/**
 * Class for downloading tiles from the Google Maps API.
 */
class FGoogleMapsAPI : public FWebMapTileAPI
{
public:
	FGoogleMapsAPI(
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
	static FString GetMapTypeStr(EGoogleMapType Type);

	FString APIKey;
	FString MapType;
};

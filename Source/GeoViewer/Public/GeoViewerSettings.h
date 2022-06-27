#pragma once

#include "CoreMinimal.h"
#include "GeoViewerSettings.generated.h"

/**
 * Configuration settings for the GeoViewer plugin.
 */
UCLASS(Config=GeoViewerSettings)
class GEOVIEWER_API UGeoViewerSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="API Keys", DisplayName="Bing Maps API Key")
	FString BingMapsAPIKey;

	UPROPERTY(Config, EditAnywhere, Category="API Keys", DisplayName="Google Maps API Key")
	FString GoogleMapsAPIKey;
};

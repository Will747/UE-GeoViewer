#include "GeoViewerEdModeConfig.h"
#include "GeoViewerEdMode.h"

void UGeoViewerEdModeConfig::Load()
{
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("MainTileSize"), TileSize, GEditorSettingsIni);

	int32 OverlaySystemInt = (int32)EBingMapType::Aerial;
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("OverlaySystem"), OverlaySystemInt, GEditorSettingsIni);
	OverlaySystem = (EOverlayMapSystem)OverlaySystemInt;

	// Bing Maps
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("BingZoomLevel"), BingMaps.ZoomLevel, GEditorSettingsIni);
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("BingTileResolution"), BingMaps.TileResolution, GEditorSettingsIni);
	GConfig->GetString(TEXT("GeoViewer"), TEXT("BingAPIKey"), BingMaps.APIKey, GEditorSettingsIni);
	
	int32 BingMapTypeInt = (int32)EBingMapType::Aerial;
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("BingType"), BingMapTypeInt, GEditorSettingsIni);
	BingMaps.Type = (EBingMapType)BingMapTypeInt;

	// Google Maps
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("GoogleZoomLevel"), GoogleMaps.ZoomLevel, GEditorSettingsIni);
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("GoogleTileResolution"), GoogleMaps.TileResolution, GEditorSettingsIni);
	GConfig->GetString(TEXT("GeoViewer"), TEXT("GoogleAPIKey"), GoogleMaps.APIKey, GEditorSettingsIni);
	
	int32 GoogleMapTypeInt = (int32)EGoogleMapType::Satellite;
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("GoogleType"), GoogleMapTypeInt, GEditorSettingsIni);
	GoogleMaps.Type = (EGoogleMapType)GoogleMapTypeInt;
}

void UGeoViewerEdModeConfig::Save()
{
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("MainTileSize"), TileSize, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("OverlaySystem"), (int32)OverlaySystem, GEditorSettingsIni);

	// Bing Maps
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("BingZoomLevel"), BingMaps.ZoomLevel, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("BingTileResolution"), BingMaps.TileResolution, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("BingType"), (int32)BingMaps.Type, GEditorSettingsIni);
	GConfig->SetString(TEXT("GeoViewer"), TEXT("BingAPIKey"), *BingMaps.APIKey, GEditorSettingsIni);

	// Google Maps
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("GoogleZoomLevel"), GoogleMaps.ZoomLevel, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("GoogleTileResolution"), GoogleMaps.TileResolution, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("GoogleType"), (int32)GoogleMaps.Type, GEditorSettingsIni);
	GConfig->SetString(TEXT("GeoViewer"), TEXT("GoogleAPIKey"), *GoogleMaps.APIKey, GEditorSettingsIni);
}

void UGeoViewerEdModeConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	// Make the overlay update with the new settings
	if (ParentMode && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		ParentMode->ResetOverlay();
	}
}

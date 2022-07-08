#include "GeoViewerEdModeConfig.h"
#include "GeoViewerEdMode.h"
#include "LandscapeInfo.h"

void UGeoViewerEdModeConfig::Load()
{
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("MainTileSize"), TileSize, GEditorSettingsIni);

	int32 OverlaySystemInt = (int32)EOverlayMapSystem::GoogleMaps;
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("OverlaySystem"), OverlaySystemInt, GEditorSettingsIni);
	OverlaySystem = (EOverlayMapSystem)OverlaySystemInt;

	// Bing Maps
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("BingZoomLevel"), BingMaps.ZoomLevel, GEditorSettingsIni);
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("BingTileResolution"), BingMaps.TileResolution, GEditorSettingsIni);
	
	int32 BingMapTypeInt = (int32)EBingMapType::Aerial;
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("BingType"), BingMapTypeInt, GEditorSettingsIni);
	BingMaps.Type = (EBingMapType)BingMapTypeInt;

	// Google Maps
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("GoogleZoomLevel"), GoogleMaps.ZoomLevel, GEditorSettingsIni);
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("GoogleTileResolution"), GoogleMaps.TileResolution, GEditorSettingsIni);
	
	int32 GoogleMapTypeInt = (int32)EGoogleMapType::Satellite;
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("GoogleType"), GoogleMapTypeInt, GEditorSettingsIni);
	GoogleMaps.Type = (EGoogleMapType)GoogleMapTypeInt;

	// Landscape
	int32 LandscapeFormatInt = (int32)ELandscapeFormat::STRM;
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("LandscapeFormat"), LandscapeFormatInt, GEditorSettingsIni);
	LandscapeFormat = (ELandscapeFormat)LandscapeFormatInt;

	int32 LandscapeReSamplingAlgorithmInt = (int32)ESamplingAlgorithm::Lanczos;
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("LandscapeAlgorithm"), LandscapeReSamplingAlgorithmInt, GEditorSettingsIni);
	LandscapeResamplingAlgorithm = (ESamplingAlgorithm)LandscapeReSamplingAlgorithmInt;
	
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("LandscapeSectionSize"), SectionSize, GEditorPerProjectIni);
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("NumberOfComponents"), NumberOfComponents, GEditorPerProjectIni);
	GConfig->GetInt(TEXT("GeoViewer"), TEXT("SectionsPerComponent"), SectionsPerComponent, GEditorPerProjectIni);

	FString LandscapeMaterialName;
	GConfig->GetString(TEXT("GeoViewer"), TEXT("LandscapeMaterial"), LandscapeMaterialName, GEditorPerProjectIni);
	if (LandscapeMaterialName != TEXT(""))
	{
		LandscapeMaterial = LoadObject<UMaterialInterface>(nullptr, *LandscapeMaterialName);
		RefreshLandscapeLayers();
	}
}

void UGeoViewerEdModeConfig::Save()
{
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("MainTileSize"), TileSize, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("OverlaySystem"), (int32)OverlaySystem, GEditorSettingsIni);

	// Bing Maps
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("BingZoomLevel"), BingMaps.ZoomLevel, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("BingTileResolution"), BingMaps.TileResolution, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("BingType"), (int32)BingMaps.Type, GEditorSettingsIni);

	// Google Maps
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("GoogleZoomLevel"), GoogleMaps.ZoomLevel, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("GoogleTileResolution"), GoogleMaps.TileResolution, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("GoogleType"), (int32)GoogleMaps.Type, GEditorSettingsIni);

	// Landscape
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("LandscapeFormat"), (int32)LandscapeFormat, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("LandscapeAlgorithm"), (int32)LandscapeResamplingAlgorithm, GEditorSettingsIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("LandscapeSectionSize"), SectionSize, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("NumberOfComponents"), NumberOfComponents, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("GeoViewer"), TEXT("SectionsPerComponent"), SectionsPerComponent, GEditorPerProjectIni);

	const FString LandscapeMaterialName = LandscapeMaterial ? LandscapeMaterial->GetPathName() : FString();
	GConfig->SetString(TEXT("GeoViewer"), TEXT("LandscapeMaterial"), *LandscapeMaterialName, GEditorPerProjectIni);
}

void UGeoViewerEdModeConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	// Make the overlay reset if a main setting has changed.
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	const bool bMainOverlaySettingChanged =
		PropertyName == GET_MEMBER_NAME_CHECKED(UGeoViewerEdModeConfig, OverlaySystem) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UGeoViewerEdModeConfig, TileSize) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UGeoViewerEdModeConfig, MaxNumberOfTiles) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(FGoogleMapsOverlayConfig, Type) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(FGoogleMapsOverlayConfig, TileResolution) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(FGoogleMapsOverlayConfig, ZoomLevel) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(FBingMapsOverlayConfig, Type) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(FBingMapsOverlayConfig, TileResolution) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(FBingMapsOverlayConfig, ZoomLevel);
		
	if (bMainOverlaySettingChanged && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		ParentMode->ResetOverlay();
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UGeoViewerEdModeConfig, Opacity))
	{
		ParentMode->UpdateOverlayOpacity();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UGeoViewerEdModeConfig, LandscapeMaterial))
	{
		RefreshLandscapeLayers();
	}
}

void UGeoViewerEdModeConfig::InitializeLandscapeLayers(const ALandscape* Landscape)
{
	// Copy the layer settings from the landscape actor if the material matches
	if (Landscape->GetLandscapeMaterial() == LandscapeMaterial)
	{
		ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();

		if (LandscapeInfo)
		{
			Layers.Empty();

			for (const FLandscapeInfoLayerSettings& LandscapeLayer : LandscapeInfo->Layers)
			{
				// Convert FLandscapeInfoLayerSettings to FLandscapeImportLayerInfo
				FLandscapeImportLayerInfo LayerInfo(LandscapeLayer.LayerName);
				LayerInfo.LayerInfo = LandscapeLayer.LayerInfoObj;
				Layers.Add(LayerInfo);
			}
		}
	}
}

void UGeoViewerEdModeConfig::RefreshLandscapeLayers()
{
	Layers.Empty();
	
	if (LandscapeMaterial)
	{
		TArray<FName> MaterialLayers = LandscapeMaterial->GetCachedExpressionData().LandscapeLayerNames;
		for (const FName& MaterialLayerName : MaterialLayers)
		{
			FLandscapeImportLayerInfo LayerInfo;
			LayerInfo.LayerName = MaterialLayerName;

			Layers.Add(LayerInfo);
		}
	}
}

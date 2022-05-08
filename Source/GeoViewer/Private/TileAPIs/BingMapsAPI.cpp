#include "TileAPIs/BingMapsAPI.h"

FBingMapsAPI::FBingMapsAPI(TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
                           AWorldReferenceSystem* ReferencingSystem):
	FWebMapTileAPI(InEdModeConfig, ReferencingSystem)
{
	if (EdModeConfigPtr.IsValid())
	{
		TileResolution = EdModeConfigPtr->BingMaps.TileResolution;
		ZoomLevel = EdModeConfigPtr->BingMaps.ZoomLevel;
		APIKey = EdModeConfigPtr->BingMaps.APIKey;
		MapType = GetMapTypeStr(EdModeConfigPtr->BingMaps.Type);
	}
}

FString FBingMapsAPI::GetMapTypeStr(const EBingMapType Type)
{
	FString TypeStr;
	switch(Type)
	{
	case EBingMapType::AerialWithLabels:
		TypeStr = "AerialWithLabels";
		break;
	case EBingMapType::Road:
		TypeStr = "Road";
		break;
	case EBingMapType::CanvasDark:
		TypeStr = "CanvasDark";
		break;
	case EBingMapType::CanvasLight:
		TypeStr = "CanvasLight";
		break;
	case EBingMapType::CanvasGray:
		TypeStr = "CanvasGray";
		break;
	default:
		TypeStr = "Aerial";
		break;
	}

	return TypeStr;
}

FString FBingMapsAPI::GetFileName(FGeographicCoordinates Coordinates) const
{
	return
		"Bing,"
		+ MapType
		+ ","
		+ FString::FromInt(TileResolution)
		+ ","
		+ FString::FromInt(ZoomLevel)
		+ ","
		+ FString::SanitizeFloat(Coordinates.Latitude)
		+ ","
		+ FString::SanitizeFloat(Coordinates.Longitude);
}

FString FBingMapsAPI::GetTileURL(FGeographicCoordinates Coordinates) const
{
	return
		"https://dev.virtualearth.net/REST/v1/Imagery/Map/" +
		MapType + "/"
		+ FString::SanitizeFloat(Coordinates.Latitude) + "," + FString::SanitizeFloat(Coordinates.Longitude) + "/"
		+ FString::FromInt(ZoomLevel)
		+ "?ms=" + FString::FromInt(TileResolution) + "," + FString::FromInt(TileResolution)
		+ "&key=" + APIKey;
}

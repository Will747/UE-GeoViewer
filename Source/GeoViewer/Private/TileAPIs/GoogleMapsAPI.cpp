#include "TileAPIs/GoogleMapsAPI.h"

FGoogleMapsAPI::FGoogleMapsAPI(TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
	AWorldReferenceSystem* ReferencingSystem, FGeoBounds TileBounds):
	FWebMapTileAPI(InEdModeConfig, ReferencingSystem, TileBounds)
{
	if (EdModeConfigPtr.IsValid())
	{
		TileResolution = EdModeConfigPtr->GoogleMaps.TileResolution;
		ZoomLevel = EdModeConfigPtr->GoogleMaps.ZoomLevel;
		APIKey = EdModeConfigPtr->GoogleMaps.APIKey;
		MapType = GetMapTypeStr(EdModeConfigPtr->GoogleMaps.Type);
	}
}

FString FGoogleMapsAPI::GetFileName(FGeographicCoordinates Coordinates) const
{
	return
	"Google,"
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

FString FGoogleMapsAPI::GetTileURL(FGeographicCoordinates Coordinates) const
{
	FString TileResolutionStr = FString::FromInt(TileResolution);
	
	return
		"https://maps.googleapis.com/maps/api/staticmap?center="
		+ FString::SanitizeFloat(Coordinates.Latitude) + "," + FString::SanitizeFloat(Coordinates.Longitude)
		+ "&zoom=" + FString::FromInt(ZoomLevel)
		+ "&size=" + TileResolutionStr + "x" + TileResolutionStr
		+ "&maptype=" + MapType
		+ "&scale=1"
		+ "&key=" + APIKey;
}

FString FGoogleMapsAPI::GetMapTypeStr(EGoogleMapType Type)
{
	FString TypeStr;
	switch(Type)
	{
	case EGoogleMapType::Hybrid:
		TypeStr = "hybrid";
		break;
	case EGoogleMapType::Terrain:
		TypeStr = "terrain";
		break;
	case EGoogleMapType::RoadMap:
		TypeStr = "roadmap";
		break;
	default:
		TypeStr = "satellite";
		break;
	}

	return TypeStr;
}

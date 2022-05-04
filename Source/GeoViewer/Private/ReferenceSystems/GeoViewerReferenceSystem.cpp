#include "ReferenceSystems/GeoViewerReferenceSystem.h"

void AGeoViewerReferenceSystem::SetGeographicalOrigin(const FGeographicCoordinates WorldOrigin)
{
	bOriginLocationInProjectedCRS = false;
	OriginLatitude = WorldOrigin.Latitude;
	OriginLongitude = WorldOrigin.Longitude;
	
	ApplySettings();
}

FGeographicCoordinates AGeoViewerReferenceSystem::GetGeographicalOrigin()
{
	FGeographicCoordinates GeoCoords;
	
	if (PlanetShape == EPlanetShape::RoundPlanet && bOriginAtPlanetCenter)
	{
		const FCartesianCoordinates ENUOrigin(0, 0, 0);
		ECEFToGeographic(ENUOrigin, GeoCoords);
	} else if (bOriginLocationInProjectedCRS)
	{
		const FCartesianCoordinates ProjectedOrigin(
			OriginProjectedCoordinatesEasting,
			OriginProjectedCoordinatesNorthing,
			OriginProjectedCoordinatesUp);
		ProjectedToGeographic(ProjectedOrigin, GeoCoords);
	} else
	{
		GeoCoords.Latitude = OriginLatitude;
		GeoCoords.Longitude = OriginLongitude;
	}
	
	return GeoCoords;
}

FString AGeoViewerReferenceSystem::EPSGToString(const uint16 EPSG)
{
	return TEXT("EPSG:") + FString::FromInt(EPSG);
}

void AGeoViewerReferenceSystem::EngineToGeographic(const FVector& EngineCoordinates,
	FGeographicCoordinates& GeographicCoordinates)
{
	if (PlanetShape == EPlanetShape::FlatPlanet)
	{
		FCartesianCoordinates ProjectedCoordinates;
		EngineToProjected(EngineCoordinates, ProjectedCoordinates);
		ProjectedToGeographic(ProjectedCoordinates, GeographicCoordinates);
	} else
	{
		//For round planets there isn't a projection so should be initially converted to ECEF coordinates
		FCartesianCoordinates ECEFCoordinates;
		EngineToECEF(EngineCoordinates, ECEFCoordinates);
		ECEFToGeographic(ECEFCoordinates, GeographicCoordinates);
	}
}

void AGeoViewerReferenceSystem::GeographicToEngine(const FGeographicCoordinates& GeographicCoordinates,
	FVector& EngineCoordinates)
{
	if (PlanetShape == EPlanetShape::FlatPlanet)
	{
		FCartesianCoordinates ProjectedCoordinates;
		GeographicToProjected(GeographicCoordinates, ProjectedCoordinates);
		ProjectedToEngine(ProjectedCoordinates, EngineCoordinates);
	} else
	{
		//For round planets there isn't a projection so should be initially converted to ECEF coordinates
		FCartesianCoordinates ECEFCoordinates;
		GeographicToECEF(GeographicCoordinates, ECEFCoordinates);
		ECEFToEngine(ECEFCoordinates, EngineCoordinates);
	}
}

void AGeoViewerReferenceSystem::GetGeographicalCoordinatesAtOffset(const FGeographicCoordinates StartPos,
                                                                   FVector2D Offset, FGeographicCoordinates& EndPos)
{
	//Convert offset to kilometers
	Offset /= 1000;
	
	//https://stackoverflow.com/a/7478827
	constexpr float EarthRadius = 6371; //km
	EndPos = StartPos;
	EndPos.Longitude += (Offset.X / EarthRadius) * (180 / PI) / FMath::Cos(StartPos.Latitude * PI / 180);
	EndPos.Latitude += (Offset.Y / EarthRadius) * (180 / PI);
}

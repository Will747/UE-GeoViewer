#include "ReferenceSystems/GeoViewerReferenceSystem.h"
#include "ReferenceSystems/WorldReferenceSystem.h"

void AGeoViewerReferenceSystem::UpdateActorSettings(AWorldReferenceSystem* WorldReferenceSystem, uint16 InProjectedEPSG)
{
	ProjectedCRS = EPSGToString(InProjectedEPSG);

	bOriginAtPlanetCenter = WorldReferenceSystem->bOriginAtPlanetCenter;
	bOriginLocationInProjectedCRS = WorldReferenceSystem->bOriginLocationInProjectedCRS;
	OriginLatitude = WorldReferenceSystem->OriginLatitude;
	OriginLongitude = WorldReferenceSystem->OriginLongitude;
	OriginAltitude = WorldReferenceSystem->OriginAltitude;

	// TODO: These would need converting to the projection of this actor
	OriginProjectedCoordinatesEasting = WorldReferenceSystem->OriginProjectedCoordinatesEasting;
	OriginProjectedCoordinatesNorthing = WorldReferenceSystem->OriginProjectedCoordinatesNorthing;
	OriginProjectedCoordinatesUp = WorldReferenceSystem->OriginProjectedCoordinatesUp;

	ApplySettings();
}

FGeographicCoordinates AGeoViewerReferenceSystem::GetGeographicalOrigin()
{
	FGeographicCoordinates GeoCoords;
	
	if (PlanetShape == EPlanetShape::RoundPlanet && bOriginAtPlanetCenter)
	{
		const FVector ENUOrigin(0, 0, 0);
		ECEFToGeographic(ENUOrigin, GeoCoords);
	} else if (bOriginLocationInProjectedCRS)
	{
		const FVector ProjectedOrigin(
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

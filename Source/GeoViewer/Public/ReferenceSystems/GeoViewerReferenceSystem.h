#pragma once

#include "CoreMinimal.h"
#include "GeoReferencingSystem.h"
#include "GeoViewerReferenceSystem.generated.h"

class AWorldReferenceSystem;

/**
 * Extends the built in geographical referencing system.
 * This actor should not be placed in the world instead use 'WorldReferenceSystem'.
 */
UCLASS()
class GEOVIEWER_API AGeoViewerReferenceSystem : public AGeoReferencingSystem
{
	GENERATED_BODY()
	
public:
	/**
	 * Sets the geographical origin of the reference system.
	 * @param WorldReferenceSystem Reference system to copy settings from.
	 * @param InProjectedEPSG Projected CRS to be used by this actor.
	 */
	void UpdateActorSettings(AWorldReferenceSystem* WorldReferenceSystem, uint16 InProjectedEPSG);

	/**
	 * @return The world origin in geographical coordinates.
	 */
	FGeographicCoordinates GetGeographicalOrigin();

	/**
	 * Creates a string with the 'EPSG:' prefix at the start.
	 * @param EPSG The EPSG code in the form of a 4 or 5 digit integer.
	 * @returm The EPSG code as a string with 'EPSG:' prefix.
	 */
	static FString EPSGToString(uint16 EPSG);
	
	/**
	 * Calculates new geographical coordinates depending on the distance in meters travelled across the surface
	 * of the earth.
	 * @param StartPos The initial geographical coordinates.
	 * @param Offset The distance North and South in meters.
	 * @param EndPos The final geographical position after the offset has been included.
	 */
	static void GetGeographicalCoordinatesAtOffset(const FGeographicCoordinates StartPos, FVector2D Offset,
		FGeographicCoordinates& EndPos);

private:
	UPROPERTY()
	uint16 EPSG;
};

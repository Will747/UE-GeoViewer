#pragma once

#include "CoreMinimal.h"
#include "GeoViewerReferenceSystem.h"
#include "WorldReferenceSystem.generated.h"

/**
 * The main coordinate reference system for the world. This actor should be
 * placed into the world and not it's parent 'GeoViewerReferenceSystem'.
 * Creates child actors to support many different projected coordinate systems.
 */
UCLASS()
class GEOVIEWER_API AWorldReferenceSystem : public AGeoViewerReferenceSystem
{
	GENERATED_BODY()
	
public:
	/**
	 * Finds existing reference system in world or creates a new one.
	 * @param WorldContext Reference to the world to get the reference system from.
	 * @return The world reference system in the world.
	 */
	static AWorldReferenceSystem* GetWorldReferenceSystem(const UObject* WorldContext);

	/**
	 * Converts from projected in the specified EPSG code to geographical.
	 * @param ProjectedCoordinates Projected coordinates to be converted.
	 * @param GeographicCoordinates Resulting geographical coordinates.
	 * @param ProjectedEPSG The projection used in the projected coordinates.
	 */
	void ProjectedToGeographicWithEPSG(
		const FVector& ProjectedCoordinates,
		FGeographicCoordinates& GeographicCoordinates,
		const uint16 ProjectedEPSG
		);

	/**
	 * Converts from geographical coordinates to projected in the specified EPSG code.
	 * @param GeographicCoordinates Geographical coordinates to be converted.
	 * @param ProjectedCoordinates Resulting projected coordinates in specified EPSG.
	 * @param ProjectedEPSG The EPSG used by the projected coordinates.
	 */
	void GeographicToProjectedWithEPSG(
		const FGeographicCoordinates& GeographicCoordinates,
		FVector& ProjectedCoordinates,
		const uint16 ProjectedEPSG
		);

private:
	/** Gets an existing child actor with the correct projection or creates a new one */
	AGeoViewerReferenceSystem* GetReferenceSystem(uint16 EPSG);
	
	/** EPSG code mapped to a reference system using the same EPSG */
	UPROPERTY()
	TMap<uint16, UChildActorComponent*> ChildReferenceSystems;
};

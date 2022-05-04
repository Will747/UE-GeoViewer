#pragma once

#include "CoreMinimal.h"
#include "GeoReferencingSystem.h"
#include "GeoViewerReferenceSystem.generated.h"

/**
 * Extends the built in geographical referencing system, including
 * being able to convert directly between Engine coordinates and
 * Geographic coordinates.
 * This actor should not be placed in the world instead use 'WorldReferenceSystem'.
 */
UCLASS()
class GEOVIEWER_API AGeoViewerReferenceSystem : public AGeoReferencingSystem
{
	GENERATED_BODY()
	
public:
	/**
	 * Sets the geographical origin of the reference system.
	 * As the origin is stored as an integer value any decimal origin would be rounded.
	 * @param WorldOrigin Origin for the reference system. Must NOT be a decimal.
	 */
	void SetGeographicalOrigin(FGeographicCoordinates WorldOrigin);

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
	 *	Converts engine coordinates to geographical coordinates.
	 *	@param EngineCoordinates The coordinates from the UE world.
	 *	@param GeographicCoordinates The geographical coordinates based on the EngineCoordinates parameter.
	 */
	void EngineToGeographic(const FVector& EngineCoordinates, FGeographicCoordinates& GeographicCoordinates);

	/**
	 *	Converts geographical coordinates to engine coordinates.
	 *	@param GeographicCoordinates The geographical coordinates to be converted.
	 *	@param EngineCoordinates The engine coordinates based on the GeographicalCoordinates parameter.
	 */
	void GeographicToEngine(const FGeographicCoordinates& GeographicCoordinates, FVector& EngineCoordinates);
	
	/**
	 * Calculates new geographical coordinates depending on the distance in meters travelled across the surface
	 * of the earth.
	 * @param StartPos The initial geographical coordinates.
	 * @param Offset The distance North and South in meters.
	 * @param EndPos The final geographical position after the offset has been included.
	 */
	static void GetGeographicalCoordinatesAtOffset(const FGeographicCoordinates StartPos, FVector2D Offset,
		FGeographicCoordinates& EndPos);
};

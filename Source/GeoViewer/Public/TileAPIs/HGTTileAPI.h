#pragma once
#include "GeoTileAPI.h"

/**
 * Class for loading STRM tiles in the HGT format.
 */
class FHGTTileAPI : public FGeoTileAPI
{
public:
	FHGTTileAPI(
		TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
		AWorldReferenceSystem* ReferencingSystem
		);

	virtual void LoadTile(FProjectedBounds TileBounds) override;
private:
	/**
	 * Opens a HGT file if one exists, returns -1 if the file
	 * does not exist.
	 * @return Index of the dataset in the DatasetsToMerge array.
	 */
	int OpenDataset(FGeographicCoordinates PositionWithinTile);
	
	/**
	 * Gets the filename of a HGT file for a specific area.
	 * @param Coordinates A position inside the HGT file.
	 * @return Filename of HGT file.
	 */
	FString GetFileName(FGeographicCoordinates Coordinates) const;

	/** Returns path to folder containing .hgt files. */
	static FString GetTerrainFolder();
	
	/**
	 * Adds 0s to the start of a string to ensure a number
	 * presented as a string has the correct number of digits.
	 * @param Number Integer to be converted to a string.
	 * @param NumOfDigits Number of digits the string should contain.
	 * @return String containing the number.
	 */
	static FString ConvertIntToString(int Number, int NumOfDigits);
	
};

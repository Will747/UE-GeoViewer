#pragma once
#include "CartesianCoordinates.h"
#include "IImageWrapper.h"
#include "SmartPointers.h"

/**
 * Class containing static functions used to help warp an image between
 * different map projections. Using the UnrealGDAL plugin to do this.
 */
class FGDALWarp
{
public:
	/**
	 * Creates a warped version of a dataset in a different projection.
	 * @param Dataset Dataset to be warped.
	 * @param CurrentCRS Current CRS of the dataset.
	 * @param FinalCRS CRS used by the returned dataset.
	 * @return Dataset reprojected to the new CRS.
	 */
	static GDALDatasetRef WarpDataset(GDALDatasetRef& Dataset, FString CurrentCRS, FString FinalCRS);
 
	/**
	 * Creates a new dataset containing raster bands created from the RawData parameter.
	 * @param RawData Raw image data to add to the dataset.
	 * @param XSize Number of rows in the image.
	 * @param YSize Number of columns in the image.
	 * @param Format Pixel format used by the image.
	 * @return GDALDataset containing the 'RawData'.
	 */
	static GDALDatasetRef CreateDataset(TArray<uint8>& RawData, int XSize, int YSize, ERGBFormat Format = ERGBFormat::RGBA);

	/**
	 * Sets the geo transform and projection on a dataset.
	 * @param Dataset Dataset to set the metadata on.
	 * @param TopCorner Projected coordinates for the top corner of the dataset.
	 * @param PixelSize Size of one pixel in comparison to the projected CRS.
	 * @param EPSG Projected coordinate system used by the dataset.
	 */
	static void SetDatasetMetaData(
		GDALDatasetRef& Dataset,
		FCartesianCoordinates TopCorner,
		const FVector2D PixelSize,
		uint16 EPSG
		);

	/**
	 * Creates a Texture2D from a raw image which can be extracted from GDALDatasets using 'GetRawImage'.
	 * @param Outer The object which owns the texture.
	 * @param RawImage The raw image the texture holds.
	 * @param SizeX The number of rows in the image.
	 * @param SizeY The number of columns in the image.
	 */
	static UTexture2D* CreateTexture2D(UObject* Outer, TArray<uint8>& RawImage, const int SizeX, const int SizeY);

	/**
	 * Converts a dataset to array of pixel data.
	 * This can take a very long time depending on the type of dataset as GDAL may
	 * need to generate and warp the image if in a virtual format.
	 * @param Dataset The dataset to extract the image from.
	 * @param OutImage The resulting raw image.
	 * @param XSize The number of columns in the dataset.
	 * @param YSize The number of rows in the dataset.
	 * @param Channels  The number of channels in the dataset.
	 * @return The raw image data from a dataset ready to be copied to a texture.
	 */
	static void GetRawImage(GDALDatasetRef& Dataset, TArray<uint8>& OutImage, int XSize, int YSize, int Channels = 4);

private:
	/** Converts to a WKT if in a valid EPSG code */
	static FString ConvertToWKT(FString CRS);
};

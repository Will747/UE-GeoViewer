#pragma once
#include "IImageWrapper.h"
#include "GDALSmartPointers.h"

/**
 * Class containing static functions used to help warp an image between
 * different map projections using GDAL.
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
	 * Crops a dataset down to the provided bounds.
	 * @param SrcDataset Dataset that needs cropping.
	 * @param TopLeft Position to crop top corner to in the projected CRS used by the dataset.
	 * @param BottomRight Position to crop bottom corner to in the projected CRS used by the dataset.
	 * @param OutFileName Path to the cropped dataset that gets created.
	 * @return Cropped dataset.
	 */
	static GDALDatasetRef CropDataset(
		GDALDataset* SrcDataset,
		FVector TopLeft,
		FVector BottomRight,
		FString& OutFileName
		);

	/**
	 * Changes the resolution of a dataset.
	 * @param SrcDataset Dataset that needs resizing.
	 * @param Resolution Dimensions the dataset should be converted to.
	 * @param OutFileName Path to the resized dataset.
	 * @return Resized dataset.
	 */
	static GDALDatasetRef ResizeDataset(
		GDALDataset* SrcDataset,
		FIntVector2 Resolution,
		FString& OutFileName
	);
	
	/**
	 * Creates a new dataset containing raster bands created from the RawData parameter.
	 * @param RawData Raw image data to add to the dataset.
	 * @param XSize Number of rows in the image.
	 * @param YSize Number of columns in the image.
	 * @param Format Pixel format used by the image.
	 * @return GDALDataset containing the 'RawData'.
	 */
	template<typename T>
	static GDALDatasetRef CreateDataset(TArray<T>& RawData, int XSize, int YSize, ERGBFormat Format = ERGBFormat::RGBA);

	/**
	 * Sets the geo transform and projection on a dataset.
	 * @param Dataset Dataset to set the metadata on.
	 * @param TopCorner Projected coordinates for the top corner of the dataset.
	 * @param PixelSize Size of one pixel in comparison to the projected CRS.
	 * @param EPSG Projected coordinate system used by the dataset.
	 */
	static void SetDatasetMetaData(
		GDALDatasetRef& Dataset,
		FVector TopCorner,
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
	template<typename T>
	static void GetRawImage(GDALDatasetRef& Dataset, TArray<T>& OutImage, int XSize, int YSize, int Channels = 4);

	/**
	 * Converts a dataset to array of pixel data.
	 * This can take a very long time depending on the type of dataset as GDAL may
	 * need to generate and warp the image if in a virtual format.
	 * @param Dataset The dataset to extract the image from.
	 * @param OutImage The resulting raw image.
	 * @return The raw image data from a dataset ready to be copied to a texture.
	 */
	template<typename T>
	static void GetRawImage(GDALDatasetRef& Dataset, TArray<T>& OutImage);


private:
	/** Converts to a WKT if in a valid EPSG code */
	static FString ConvertToWKT(FString CRS);
	static FString ConvertToWKT(uint16 EPSGInt);

	/**
	 * Runs the GDALTranslate function.
	 * @param Dataset Source dataset.
	 * @param Parameters Translate parameters.
	 * @param OutFileName Random filename of translated dataset saved in memory.
	 */
	static GDALDatasetRef TranslateDataset(
		GDALDataset* Dataset,
		TArray<FString>& Parameters,
		FString& OutFileName
		);

	static FString ConvertToFString(char* Text);
};

template <typename T>
GDALDatasetRef FGDALWarp::CreateDataset(TArray<T>& RawData, int XSize, int YSize, ERGBFormat Format)
{
	int ChannelNum = 4;
	if (Format == ERGBFormat::Gray)
	{
		ChannelNum = 1;
	}
	
	const mergetiff::RasterData<T> RasterData(
		RawData.GetData(),
		ChannelNum,
		YSize,
		XSize,
		true
		);
	
	return mergetiff::DatasetManagement::datasetFromRaster(RasterData);
}

template <typename T>
void FGDALWarp::GetRawImage(GDALDatasetRef& Dataset, TArray<T>& OutImage, int XSize, int YSize, int Channels)
{
	constexpr T Element{};
	OutImage.Init(Element, XSize * YSize * Channels);
	mergetiff::RasterData RasterData(OutImage.GetData(), Channels, YSize, XSize, true);
	mergetiff::RasterIO::readDataset<T>(Dataset, RasterData);
}

template <typename T>
void FGDALWarp::GetRawImage(GDALDatasetRef& Dataset, TArray<T>& OutImage)
{
	const int XSize = Dataset->GetRasterXSize();
	const int YSize = Dataset->GetRasterYSize();
	const int Channels = Dataset->GetRasterCount();

	GetRawImage(Dataset, OutImage, XSize, YSize, Channels);
}

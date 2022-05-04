#include "GDALWarp.h"
#include <gdalwarper.h>
#include "GDALHelpers.h"
#include "GeoViewer.h"

GDALDatasetRef FGDALWarp::WarpDataset(GDALDatasetRef& Dataset, FString CurrentCRS, FString FinalCRS)
{
	const FString SrcWKT = ConvertToWKT(CurrentCRS);
	const FString DstWKT = ConvertToWKT(FinalCRS);

	GDALDataset* DstDataset = (GDALDataset*) GDALAutoCreateWarpedVRT(
		Dataset.Get(),  TCHAR_TO_UTF8(*SrcWKT), TCHAR_TO_UTF8(*DstWKT), GRA_Lanczos, 1, NULL);

	return GDALDatasetRef(DstDataset);
}

FString FGDALWarp::ConvertToWKT(FString CRS)
{
	// If the string is the correct length try and convert it
	//EPSG:3857 - The first part is 5 digits plus 4 or 5 digits for the EPSG code
	if (CRS.Len() == 9 || CRS.Len() == 10)
	{
		const FString EPSGCode = CRS.Mid(5); //Remove EPSG: prefix
		const uint16 EPSGInt = FCString::Atoi(*EPSGCode);
		return GDALHelpers::WktFromEPSG(EPSGInt);
	}

	// If the wrong length it may be invalid or already in the form of a WKT
	return CRS;
}

GDALDatasetRef FGDALWarp::CreateDataset(TArray<uint8>& RawData, const int XSize, const int YSize, const ERGBFormat Format)
{
	int ChannelNum = 4;
	if (Format == ERGBFormat::Gray)
	{
		ChannelNum = 1;
	}
	
	const mergetiff::RasterData<uint8> RasterData(
		RawData.GetData(),
		ChannelNum,
		YSize,
		XSize,
		true
		);
	
	return mergetiff::DatasetManagement::datasetFromRaster(RasterData);
}

void FGDALWarp::SetDatasetMetaData(GDALDatasetRef& Dataset, FCartesianCoordinates TopCorner, const FVector2D PixelSize,
                                   const uint16 EPSG)
{
	//Set tile projection
	const FString Wkt = GDALHelpers::WktFromEPSG(EPSG);
	GDALSetProjection(Dataset.Get(), TCHAR_TO_UTF8(*Wkt));

	//Set geo transform
	double GeoTransform[6];
	GeoTransform[0] = TopCorner.X;
	GeoTransform[3] = TopCorner.Y;

	GeoTransform[2] = 0;
	GeoTransform[4] = 0;
	
	GeoTransform[1] = PixelSize.X;
	GeoTransform[5] = -PixelSize.Y; //Negative for north up images
	
	GDALSetGeoTransform(Dataset.Get(), GeoTransform);
}

UTexture2D* FGDALWarp::CreateTexture2D(UObject* Outer, TArray<uint8>& RawImage, const int SizeX, const int SizeY)
{
	UTexture2D* Texture = nullptr;

	 if (SizeX > 0 && SizeY > 0)
	 {
		constexpr EPixelFormat PixelFormat = PF_B8G8R8A8;
		
		// Create new texture
		Texture = NewObject<UTexture2D>(Outer);

		// Setup platform data
		Texture->PlatformData = new FTexturePlatformData();
		Texture->PlatformData->SizeX = SizeX;
		Texture->PlatformData->SizeY = SizeY;
		Texture->PlatformData->PixelFormat = PixelFormat;

		// Create mip ready to transfer raw image from dataset
		FTexture2DMipMap* Mip = new FTexture2DMipMap();
		Texture->PlatformData->Mips.Add(Mip);
		Mip->SizeX = SizeX;
		Mip->SizeY = SizeY;
		
		// Allocate and copy the image
		Mip->BulkData.Lock(LOCK_READ_WRITE);
		void* TextureData = Mip->BulkData.Realloc(RawImage.Num());
		FMemory::Memcpy(TextureData, RawImage.GetData(), RawImage.Num());
		Mip->BulkData.Unlock();

		Texture->UpdateResource();
	 } else
	 {
		 UE_LOG(LogGeoViewer, Warning, TEXT("Invalid raster data in dataset"))
	 }
	
	return Texture;
}

void FGDALWarp::GetRawImage(GDALDatasetRef& Dataset, TArray<uint8>& OutImage, int XSize, int YSize, int Channels)
{
	mergetiff::RasterData<uint8> RasterData =
		GDALHelpers::AllocateAndWrap<uint8>(OutImage, Channels, YSize, XSize, 255);
	mergetiff::RasterIO::readDataset<uint8>(Dataset, RasterData);
}

﻿#include "GDALWarp.h"
#include "GeoViewer.h"

GDALDatasetRef FGDALWarp::WarpDataset(const GDALDatasetRef& Dataset, const FString CurrentCRS, const FString FinalCRS)
{
	const FString SrcWKT = ConvertToWKT(CurrentCRS);
	const FString DstWKT = ConvertToWKT(FinalCRS);

	GDALDataset* DstDataset = (GDALDataset*) GDALAutoCreateWarpedVRT(
		Dataset.Get(),
		TCHAR_TO_UTF8(*SrcWKT),
		TCHAR_TO_UTF8(*DstWKT),
		GRA_Lanczos,
		1,
		NULL
		);

	return GDALDatasetRef(DstDataset);
}

GDALDatasetRef FGDALWarp::CropDataset(
	GDALDataset* SrcDataset,
	const FVector TopLeft,
	const FVector BottomRight,
	FString& OutFileName
	)
{
	TArray<FString> TranslateParameters;
	TranslateParameters.Add("-projwin");
	TranslateParameters.Add(FString::SanitizeFloat(BottomRight.X));
	TranslateParameters.Add(FString::SanitizeFloat(BottomRight.Y));
	TranslateParameters.Add(FString::SanitizeFloat(TopLeft.X));
	TranslateParameters.Add(FString::SanitizeFloat(TopLeft.Y));
	
	return TranslateDataset(SrcDataset, TranslateParameters, OutFileName);
}

void FGDALWarp::DeleteVRTDatasets(TArray<FString>& DatasetPaths)
{
	GDALDriver* Driver = (GDALDriver*)GDALGetDriverByName("VRT");

	for (FString DatasetPath : DatasetPaths)
	{
		Driver->Delete(TCHAR_TO_UTF8(*DatasetPath));
	}
}

GDALDatasetRef FGDALWarp::MergeDatasets(TArray<GDALDatasetRef>& Datasets)
{
	TArray<GDALDataset*> DatasetPtrs;

	for (GDALDatasetRef& DatasetRef : Datasets)
	{
		DatasetPtrs.Add(DatasetRef.Get());
	}

	return MergeDatasets(DatasetPtrs);
}

GDALDatasetRef FGDALWarp::MergeDatasets(TArray<GDALDataset*>& Datasets)
{
	std::vector<GDALDataset*> DatasetsVector;

	for (GDALDataset* Dataset : Datasets)
	{
		DatasetsVector.push_back(Dataset);
	}

	int OutputError = FALSE;

	GDALAllRegister();
	
	GDALDataset* MergedDataset = (GDALDataset*)GDALBuildVRT(
			"",
			DatasetsVector.size(),
			(GDALDatasetH*)DatasetsVector.data(),
			nullptr,
			nullptr,
			&OutputError
			);

	return GDALDatasetRef(MergedDataset);
}

GDALDatasetRef FGDALWarp::ResizeDataset(
	GDALDataset* SrcDataset,
	const FIntVector2 Resolution,
	FString& OutFileName,
	const ESamplingAlgorithm Algorithm /*=ESamplingAlgorithm::Lanczos*/)
{
	TArray<FString> TranslateParameters;
	TranslateParameters.Add("-outsize");
	TranslateParameters.Add(FString::FromInt(Resolution.X));
	TranslateParameters.Add(FString::FromInt(Resolution.Y));
	TranslateParameters.Add("-r");
	TranslateParameters.Add(GetSamplingParameter(Algorithm));
	
	return TranslateDataset(SrcDataset, TranslateParameters, OutFileName);
}

FString FGDALWarp::ConvertToWKT(FString CRS)
{
	// If the string is the correct length try and convert it
	//EPSG:3857 - The first part is 5 digits plus 4 or 5 digits for the EPSG code
	if (CRS.Len() == 9 || CRS.Len() == 10)
	{
		const FString EPSGCode = CRS.Mid(5); //Remove EPSG: prefix
		const uint16 EPSGInt = FCString::Atoi(*EPSGCode);
		return ConvertToWKT(EPSGInt);
	}

	// If the wrong length it may be invalid or already in the form of a WKT
	return CRS;
}

FString FGDALWarp::ConvertToFString(char* Text)
{
	const CPLStringRef TextRef(Text);
	return UTF8_TO_TCHAR(TextRef.Get());
}

void FGDALWarp::SetDatasetMetaData(GDALDatasetRef& Dataset, FVector TopCorner, const FVector2D PixelSize,
                                   const uint16 EPSG)
{
	//Set tile projection
	const FString Wkt = ConvertToWKT(EPSG);
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

		// Create platform data
		FTexturePlatformData* PlatformData = new FTexturePlatformData();
	 	Texture->SetPlatformData(PlatformData);

	 	// Setup platform data
		PlatformData->SizeX = SizeX;
		PlatformData->SizeY = SizeY;
		PlatformData->PixelFormat = PixelFormat;

		// Create mip ready to transfer raw image from dataset
		FTexture2DMipMap* Mip = new FTexture2DMipMap();
		PlatformData->Mips.Add(Mip);
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

FString FGDALWarp::ConvertToWKT(const uint16 EPSGInt)
{
	OGRSpatialReference SpatialReference;
	SpatialReference.importFromEPSG(EPSGInt);

	char* Wkt;
	SpatialReference.exportToWkt(&Wkt);
	
	return ConvertToFString(Wkt);
}

GDALDatasetRef FGDALWarp::TranslateDataset(
	GDALDataset* Dataset,
	TArray<FString>& Parameters,
	FString& OutFileName
	)
{
	mergetiff::ArgsArray ParametersChar;
	for (FString Parameter : Parameters)
	{
		ParametersChar.add(TCHAR_TO_UTF8(*Parameter));
	}
	
	GDALTranslateOptions* Options = GDALTranslateOptionsNew(ParametersChar.get(), nullptr);

	const FGuid DatasetGuid = FGuid::NewGuid();
	OutFileName = "/vsimem/" + DatasetGuid.ToString() + ".vrt";
	
	GDALDataset* TranslatedDataset =
		(GDALDataset*)GDALTranslate(TCHAR_TO_UTF8(*OutFileName), Dataset, Options, NULL);
	GDALTranslateOptionsFree(Options);

	return GDALDatasetRef(TranslatedDataset);
}

FString FGDALWarp::GetSamplingParameter(ESamplingAlgorithm Algorithm)
{
	switch (Algorithm)
	{
	case ESamplingAlgorithm::Nearest: return "nearest";
	case ESamplingAlgorithm::Average: return "average";
	case ESamplingAlgorithm::Rms: return "rms";
	case ESamplingAlgorithm::Bilinear: return "bilinear";
	case ESamplingAlgorithm::Cubic: return "cubic";
	case ESamplingAlgorithm::CubicSpline: return "cubicspline";
	case ESamplingAlgorithm::Mode: return "mode";
	default: return "lanczos";
	}
}

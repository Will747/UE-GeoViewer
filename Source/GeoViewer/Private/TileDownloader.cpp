#include "TileDownloader.h"
#include "GDALWarp.h"
#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpResponse.h"
#include "TileAPIS/GeoTileAPI.h"

FTileDownloader::FTileDownloader(): FinalDataset(nullptr), EPSG(0)
{
}

FTileDownloader::~FTileDownloader()
{
}

void FTileDownloader::SetMetaData(FVector InTopCorner, FVector2D InPixelSize, uint16 InEPSG)
{
	TopCorner = InTopCorner;
	PixelSize = InPixelSize;
	EPSG = InEPSG;
}

//Based on FWebImage
bool FTileDownloader::BeginDownload(FString InURL, FString InFileName)
{
	FileName = InFileName;
	
	if (InURL.IsEmpty())
	{
		return false;
	}

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(InURL);
	HttpRequest->SetHeader(TEXT("Accept"), TEXT("image/png, image/x-png, image/jpeg; q=0.8, image/vnd.microsoft.icon, image/x-icon, image/bmp, image/*; q=0.5, image/webp; q=0.0"));
	HttpRequest->OnProcessRequestComplete().BindSP(this, &FTileDownloader::DownloadFinished);

	if (!HttpRequest->ProcessRequest())
	{
		return false;
	}

	PendingRequest = HttpRequest;
	return true;
}

void FTileDownloader::DownloadFinished(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	// clear our handle to the request
	PendingRequest.Reset();

	// get the request URL
	check(HttpRequest.IsValid()); // this should be valid, we did just send a request...
	if (HttpRequest->OnProcessRequestComplete().IsBound())
	{
		HttpRequest->OnProcessRequestComplete().Unbind();
	}

	// build an image wrapper for this type
	static const FName MODULE_IMAGE_WRAPPER("ImageWrapper");
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(MODULE_IMAGE_WRAPPER);
	
	const TArray<uint8>& Content = HttpResponse->GetContent();
	const EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(Content.GetData(), Content.Num());

	const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

	// Parse the content
	if (!ImageWrapper || !ImageWrapper->SetCompressed(Content.GetData(), Content.Num()))
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, "Map Overlay: Failed to download image");
		OnDownloaded.Execute(this);
		return;
	}

	// Get raw image data from ImageWrapper
	TArray<uint8> RawImageData;
	ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawImageData);

	const int XSize = ImageWrapper->GetWidth();
	const int YSize = ImageWrapper->GetHeight();

	GDALAllRegister();
	
	// Create and store dataset
	const char* DriverName = "GTiff";
	GDALDriver* GTiffDriver = GetGDALDriverManager()->GetDriverByName(DriverName);
	check(GTiffDriver)

	GDALDataType GdalType = mergetiff::DatatypeConversion::primitiveToGdal<uint8>();
	const FString CacheFolder = FGeoTileAPI::GetCacheFolderPath();
	const FString FilePath = CacheFolder + FileName + TEXT(".tif");
	GDALDataset* SavedDataset = GTiffDriver->Create(
		TCHAR_TO_UTF8(*FilePath),
		XSize,
		YSize,
		4,
		GdalType,
		nullptr
		);

	GDALDatasetRef DownloadedDataset(SavedDataset);

	FGDALWarp::SetDatasetMetaData(DownloadedDataset, TopCorner, PixelSize, EPSG);
	
	const mergetiff::RasterData<uint8> RasterData(
			RawImageData.GetData(),
			4,
			YSize,
			XSize,
			true
			);
	
	if (mergetiff::RasterIO::writeDataset(DownloadedDataset, RasterData))
	{
		// Close the asset to ensure it's saved then reopen it in read only mode
		//GDALDataset* OldDataset = DownloadedDataset.Release();
		//GDALClose(OldDataset);

		FinalDataset = DownloadedDataset.Release();
		OnDownloaded.Execute(this);
	}
}

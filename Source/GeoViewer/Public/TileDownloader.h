#pragma once

#include "CoreMinimal.h"
#include "GeoViewerEdModeConfig.h"
#include "GDALSmartPointers.h"
#include "Interfaces/IHttpRequest.h"

/**
 * This class is based on FWebImage and is used to handle downloading
 * an image through a HTTP request and then return as a GDALDatasetRef.
 */
class FTileDownloader : public TSharedFromThis<FTileDownloader>
{
public:
	DECLARE_DELEGATE_OneParam(FOnDownloaded, const FTileDownloader*);
	
	FTileDownloader();
	~FTileDownloader();

	/** Starts downloading the image from the URL provided */
	bool BeginDownload(FString InURL, FString InFileName);

	/** Sets the geographic information ready for the dataset */
	void SetMetaData(
		const FVector InTopCorner,
		const FVector2D InPixelSize,
		const uint16 InEPSG
		);
	
	FOnDownloaded OnDownloaded;

	GDALDataset* FinalDataset;
private:
	void DownloadFinished(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

	/** Any pending request */
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> PendingRequest;

	/** Details of the tile */
	TWeakObjectPtr<UGeoViewerEdModeConfig> EdModeConfig;

	/** Metadata to be added to the dataset */
	FVector TopCorner;
	FVector2D PixelSize;
	uint16 EPSG;

	FString FileName;
};

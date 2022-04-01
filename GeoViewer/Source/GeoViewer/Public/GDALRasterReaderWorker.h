#pragma once
#include "SmartPointers.h"

/**
 * Extracts the raw image data from a GDALDataset.
 * Uses its own thread as some datasets can take a while
 * depending on if they have been warped or formed of many
 * smaller datasets.
 */
class FGDALRasterReaderWorker : public FRunnable
{
public:
	FGDALRasterReaderWorker(GDALDataset* InDataset, TArray<uint8>& OutRawImage);

	virtual ~FGDALRasterReaderWorker() override;

	// FRunnable Interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	// End of FRunnable Interface

	/** Dimensions of dataset */
	int SizeX;
	int SizeY;

	/** True once the thread has finished extracting raw image data from the dataset */
	bool bDone;
private:
	TArray<uint8>* RawImage;
	GDALDatasetRef Dataset;

	FRunnableThread* Thread;
};

#include "GDALRasterReaderWorker.h"
#include "GDALWarp.h"

FGDALRasterReaderWorker::FGDALRasterReaderWorker(
	GDALDataset* InDataset, TArray<uint8>& OutRawImage)
{
	Dataset = GDALDatasetRef(InDataset);
	RawImage = &OutRawImage;

	bDone = false;
	Thread = FRunnableThread::Create(this, TEXT("GDALDataset to Raw Image Worker"));
}

FGDALRasterReaderWorker::~FGDALRasterReaderWorker()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

bool FGDALRasterReaderWorker::Init()
{
	bDone = false;
	return true;
}

uint32 FGDALRasterReaderWorker::Run()
{
	if (Dataset.IsValid())
	{
		GDALDataset* DatasetPtr = Dataset.Get();
		SizeX = DatasetPtr->GetRasterXSize();
		SizeY = DatasetPtr->GetRasterYSize();
		const int ChannelNum = DatasetPtr->GetRasterCount();

		FGDALWarp::GetRawImage(Dataset, *RawImage, SizeX, SizeY, ChannelNum);
	}
	
	return 0;
}

void FGDALRasterReaderWorker::Exit()
{
	bDone = true;
}



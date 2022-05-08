#include "TileAPIS/GeoTileAPI.h"
#include "Interfaces/IPluginManager.h"

FGeoTileAPI::FGeoTileAPI(
	TWeakObjectPtr<UGeoViewerEdModeConfig> InEdModeConfig,
	AWorldReferenceSystem* ReferencingSystem
	)
{
	EdModeConfigPtr = InEdModeConfig;
	TileReferenceSystem = ReferencingSystem;
}

FGeoTileAPI::~FGeoTileAPI()
{
	DatasetsToMerge.Empty();
	CachedDatasets.Empty();
}

FString FGeoTileAPI::GetCacheFolderPath()
{
	const TSharedPtr<IPlugin> PluginManager = IPluginManager::Get().FindPlugin(TEXT("GeoViewer"));
	return PluginManager->GetBaseDir() + TEXT("/Resources/CachedTiles/");
}

void FGeoTileAPI::TriggerOnCompleted(GDALDataset* Dataset) const
{
	OnComplete.ExecuteIfBound(Dataset);
}

GDALDataset* FGeoTileAPI::MergeDatasets()
{
	std::vector<GDALDataset*> DatasetsVector;

	for (GDALDataset* Dataset : DatasetsToMerge)
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

	EmptyDatasetsToMerge();
	
	if (MergedDataset)
	{
		return MergedDataset;
	}

	return nullptr;
}

void FGeoTileAPI::EmptyDatasetsToMerge()
{
	for (GDALDataset* Dataset : DatasetsToMerge)
	{
		GDALClose(Dataset);
		Dataset = nullptr;
	}

	DatasetsToMerge.Empty();
}

#include "LandscapeImporter.h"

#include "GDALWarp.h"
#include "GeoViewer.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeStreamingProxy.h"
#include "SWeightMapImportDlg.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/GameplayStatics.h"
#include "TileAPIs/HGTTileAPI.h"
#include "TileAPIs/MapBoxTerrain.h"

#define LOCTEXT_NAMESPACE "GeoViewerLandscapeImporter"

FLandscapeImporter::FLandscapeImporter(): World(nullptr), EdModeConfig(nullptr)
{
}

void FLandscapeImporter::Initialize(UWorld* InWorld, UGeoViewerEdModeConfig* InEdModeConfig)
{
	World = InWorld;
	EdModeConfig = InEdModeConfig;
}

void FLandscapeImporter::LoadTile(FVector LandscapePosition)
{
	if (EdModeConfig && World)
	{
		// Side length of a landscape tile in centimeters
		const float TileLength = GetNumOfQuadsOneAxis() * 100;

		// Calculate potential bottom corner position for landscape
		FIntPoint IntPosition;
		IntPosition.X = FMath::Floor(LandscapePosition.X / TileLength);
		IntPosition.Y = FMath::Floor(LandscapePosition.Y / TileLength);

		// Calculate bounds for new tile
		FVector BottomCorner;
		BottomCorner.X = IntPosition.X * TileLength;
		BottomCorner.Y = IntPosition.Y * TileLength;
		BottomCorner.Z = 0;

		const FVector TopCorner = BottomCorner + TileLength;
		CurrentSectionOffset = IntPosition * GetNumOfQuadsOneAxis();

		// Ensure this landscape tile doesn't already exist
		const ALandscape* LandscapeActor = GetLandscapeActor();
		const ULandscapeInfo* LandscapeInfo = LandscapeActor->GetLandscapeInfo();

		if (LandscapeInfo->XYtoComponentMap.Contains(CurrentSectionOffset))
		{
			return;
		}

		// Convert to geographical bounds
		AWorldReferenceSystem* ReferenceSystem = AWorldReferenceSystem::GetWorldReferenceSystem(World);
		if (!ReferenceSystem) return;
		
		// Add an additional 500 meters to be cropped off later on
		// This should make the joins between landscape actors less noticeable
		const FVector TopCornerTile = TopCorner + (500 * 100);
		const FVector BottomCornerTile = BottomCorner - (500 * 100);
		
		FProjectedBounds TileBounds;
		ReferenceSystem->EngineToProjected(TopCornerTile, TileBounds.TopLeft);
		ReferenceSystem->EngineToProjected(BottomCornerTile, TileBounds.BottomRight);
		
		// Get weight maps
		WeightMaps.Empty();
		ImportWeightMap(WeightMaps, TileBounds);

		const TSharedRef<FGeoTileAPI> TileAPI = GetTileAPI();
		TileAPI->LoadTile(TileBounds);
		
		CachedTileAPI = TileAPI;
	}
}

float FLandscapeImporter::GetNumOfQuadsOneAxis() const
{
	if (EdModeConfig)
	{
		const float ComponentNumSqrt = FMath::Sqrt((float)EdModeConfig->NumberOfComponents);
		return ComponentNumSqrt * EdModeConfig->SectionSize * EdModeConfig->SectionsPerComponent;
	}

	return -1;
}

float FLandscapeImporter::GetNumOfVerticesOneAxis() const
{
	return GetNumOfQuadsOneAxis() + 1;
}

FVector FLandscapeImporter::GetLandscapeScale() const
{
	const float ZScale = (MountEverestHeight/2 * 100)/512;
	return FVector(100, 100, ZScale);
}

void FLandscapeImporter::OnTileDataLoaded(GDALDataset* Dataset)
{
	if (Dataset)
	{
		GDALDatasetRef DatasetRef(Dataset);

		const float HeightScale = UINT16_MAX / (MountEverestHeight/2);
		constexpr float SeaLevelOffset = UINT16_MAX / 2;
		
		TArray<uint16> HeightData;
		
		// Read height data from dataset
		const GDALDataType DataType = Dataset->GetRasterBand(1)->GetRasterDataType();

		// Mapbox datasets use floats and HGT uses int16
		if (DataType == GDT_Float32)
		{
			TArray<float> HeightDataFloat;
			FGDALWarp::GetRawImage(DatasetRef, HeightDataFloat);

			// Convert and resize height data to uint16
			HeightData.Init(0, HeightDataFloat.Num());
			for (int i = 0; i < HeightDataFloat.Num(); i++)
			{
				HeightData[i] = SeaLevelOffset + (HeightDataFloat[i] * HeightScale);
			}
		} else
		{
			TArray<int16> HeightDataSigned;
			FGDALWarp::GetRawImage(DatasetRef, HeightDataSigned);

			// Convert and resize height data to uint16
			HeightData.Init(0, HeightDataSigned.Num());
			for (int i = 0; i < HeightDataSigned.Num(); i++)
			{
				HeightData[i] = SeaLevelOffset + (HeightDataSigned[i] * HeightScale);
			}
		}

		// Tile size in pixels
		const float FinalTileLength = GetNumOfVerticesOneAxis();
		const float TileLength = FinalTileLength + 1000;
		
		// Height data should be a square
		const int SideLength = FMath::Sqrt((float)HeightData.Num());

		// Create new dataset so the height data can be resized
		const GDALDatasetRef HeightDataset =
			FGDALWarp::CreateDataset(HeightData, SideLength, SideLength, ERGBFormat::Gray);

		TArray<FString> FilesToDelete;
		
		// Resize height map
		FString DatasetFilePath;
		GDALDatasetRef ResizedHeightDataset = FGDALWarp::ResizeDataset(
			HeightDataset.Get(),
			FIntVector2(TileLength, TileLength),
			DatasetFilePath
			);
		FilesToDelete.Add(DatasetFilePath);

		TArray<uint16> HeightDataResized;
		FGDALWarp::GetRawImage(ResizedHeightDataset, HeightDataResized);
		
		TArray<uint16> HeightDataCropped;
		HeightDataCropped.Reserve(FinalTileLength * FinalTileLength);

		// Crop height data down to final size
		const int MinXY = 500;
		for (int y = 0; y < FinalTileLength; y++)
		{
			for (int x = 0; x < FinalTileLength; x++)
			{
				const int Index = x + MinXY + (y * TileLength) + (MinXY * TileLength);
				HeightDataCropped.Add(HeightDataResized[Index]);
			}
		}

		if (HeightDataCropped.Num() == FinalTileLength * FinalTileLength)
		{
			CreateLandscapeProxy(HeightDataCropped);
		}
		
		// Remove all datasets from memory
		FGDALWarp::DeleteVRTDatasets(FilesToDelete);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load height data files"));
	}

	CachedTileAPI.Reset();
}

void FLandscapeImporter::CreateLandscapeProxy(const TArray<uint16>& HeightData) const
{
	// Prepare weight maps for landscape
	TArray<FLandscapeImportLayerInfo> LandscapeImportLayers;
	
	if (EdModeConfig->Layers.Num() <= WeightMaps.Num())
	{
		for (int LayerIdx = 0; LayerIdx < EdModeConfig->Layers.Num(); LayerIdx++)
		{
			FLandscapeImportLayerInfo LayerInfo = EdModeConfig->Layers[LayerIdx];

			if (LayerInfo.LayerInfo.Get())
			{
				LayerInfo.LayerData = WeightMaps[LayerIdx];
				LandscapeImportLayers.Add(LayerInfo);
			}
			else
			{
				// Without a layer info object this layer can't be added
				FText LayerNameText = FText::FromName(LayerInfo.LayerName);
				EAppReturnType::Type MessageResponse = FMessageDialog::Open(
					EAppMsgType::OkCancel,
					FText::Format(LOCTEXT("LayerInfoMissing", "Missing layer info for '{0}'"), LayerNameText)
				);

				if (MessageResponse == EAppReturnType::Cancel)
				{
					return;
				}
			}
		}	
	}
	
	// Create Landscape Proxy
	const ALandscape* LandscapeActor = GetLandscapeActor();
	
	ALandscapeStreamingProxy* LandscapeProxy = World->SpawnActor<ALandscapeStreamingProxy>();
	LandscapeProxy->LandscapeMaterial = EdModeConfig->LandscapeMaterial;
	LandscapeProxy->SetLandscapeGuid(FGuid::NewGuid());
	LandscapeProxy->SetActorScale3D(GetLandscapeScale());
	LandscapeProxy->LandscapeSectionOffset = CurrentSectionOffset;
	LandscapeActor->GetLandscapeInfo()->FixupProxiesTransform();
	
	TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;
	
	HeightmapDataPerLayers.Add(FGuid(), HeightData);
	MaterialLayerDataPerLayer.Add(FGuid(), LandscapeImportLayers);

	// Height data should be a square
	const int SideLength = FMath::Sqrt((float)HeightData.Num());

	const FGuid LandscapeGuid = LandscapeActor->GetLandscapeGuid();

	const int MinX = CurrentSectionOffset.X;
	const int MinY = CurrentSectionOffset.Y;
	const int MaxX = MinX + SideLength - 1;
	const int MaxY = MinY + SideLength - 1;

	LandscapeProxy->Import(
		LandscapeGuid,
		MinX,
		MinY,
		MaxX,
		MaxY,
		EdModeConfig->SectionsPerComponent,
		EdModeConfig->SectionSize,
		HeightmapDataPerLayers,
		*FString(),
		MaterialLayerDataPerLayer,
		ELandscapeImportAlphamapType::Additive
		);
}

ALandscape* FLandscapeImporter::GetLandscapeActor() const
{
	ALandscape* LandscapeActor = nullptr;
	const FGuid LandscapeGuid = FGuid::NewGuid();
	
	if (World)
	{
		// Search world for landscape actors
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Actors);

		if (Actors.Num() > 0)
		{
			LandscapeActor = Cast<ALandscape>(Actors[0]);
		} else if (EdModeConfig)
		{
			// As no landscape exists in the world create a new one
			LandscapeActor = World->SpawnActor<ALandscape>();
			
			LandscapeActor->LandscapeMaterial = EdModeConfig->LandscapeMaterial;
			LandscapeActor->ComponentSizeQuads = EdModeConfig->SectionSize * EdModeConfig->SectionsPerComponent;
			LandscapeActor->NumSubsections = EdModeConfig->SectionsPerComponent;
			LandscapeActor->SubsectionSizeQuads = EdModeConfig->SectionSize;
			LandscapeActor->SetLandscapeGuid(LandscapeGuid);
			LandscapeActor->SetActorScale3D(GetLandscapeScale());

			LandscapeActor->CreateLandscapeInfo();
		}
	}

	return LandscapeActor;
}

void FLandscapeImporter::ImportWeightMap(TArray<TArray<uint8>>& RawData, FProjectedBounds Bounds) const
{
	// Create new window to select weight map files
	TSharedRef<SWindow> WeightMapWindow = SNew(SWindow)
		.Title(LOCTEXT("WeightMapWindowTitle", "Import Weight Maps"))
		.ClientSize(FVector2D(800, 200))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	// Convert projected bounds to geo for weight map importer widget
	AWorldReferenceSystem* ReferenceSystem = AWorldReferenceSystem::GetWorldReferenceSystem(World);
	FGeoBounds GeoBounds;
	if (ReferenceSystem)
	{
		GeoBounds = Bounds.ConvertToGeoBounds(ReferenceSystem);
	}
	
	TSharedRef<SWeightMapImportDlg> ImportDlg = SNew(SWeightMapImportDlg, WeightMapWindow, GeoBounds);
	WeightMapWindow->SetContent(ImportDlg);

	GEditor->EditorAddModalWindow(WeightMapWindow);

	TArray<FString> Files = ImportDlg->GetFilePaths();
	if (Files.Num() <= 0) return;
	
	// Open all weight maps
	TArray<GDALDatasetRef> Datasets;
	for (FString& File : Files)
	{
		GDALDataset* Dataset = (GDALDataset*)GDALOpen(TCHAR_TO_UTF8(*File), GA_ReadOnly);
		Datasets.Add(GDALDatasetRef(Dataset));
	}

	// Merge datasets into one
	GDALDatasetRef MergedDataset = FGDALWarp::MergeDatasets(Datasets);
	if (!MergedDataset.IsValid()) return;
	
	if (MergedDataset->GetRasterCount() != 1)
	{
		UE_LOG(LogGeoViewer, Error, TEXT("Weightmaps should only contain one channel and must be greyscale!"))
	}

	// Extract to raw image
	TArray<uint8> RawMergedWeightMap;
	FGDALWarp::GetRawImage(MergedDataset, RawMergedWeightMap);

	const int MergedXSize = MergedDataset->GetRasterXSize();
	const int MergedYSize = MergedDataset->GetRasterYSize();
	double MergedGeoTransform[6];
	MergedDataset->GetGeoTransform(MergedGeoTransform);
	const char* MergedProjection = MergedDataset->GetProjectionRef();
	
	// Find the max value in the weight map this should be to the number of material layers
	int NumOfLayers = 0;
	for (int i = 0; i < RawMergedWeightMap.Num(); i++)
	{
		// Reset any max values to 0
		if (RawMergedWeightMap[i] == UINT8_MAX)
		{
			RawMergedWeightMap[i] = 0;
		}
		
		if (RawMergedWeightMap[i] > NumOfLayers)
		{
			NumOfLayers = RawMergedWeightMap[i];
		}
	}

	// Separate raw image into layers
	TArray<TArray<uint8>> Layers;

	const float TileLength = GetNumOfVerticesOneAxis();
	FIntVector2 RequiredResolution(TileLength, TileLength);
	
	// Create each layer
	for (int LayerIdx = 0; LayerIdx < NumOfLayers; LayerIdx++)
	{
		TArray<uint8> Layer;
		Layer.Init(0, RawMergedWeightMap.Num());

		// Go through each pixel and if the pixel value matches the layer idx make that pixel max weight
		for (int i = 0; i < RawMergedWeightMap.Num(); i++)
		{
			if (RawMergedWeightMap[i] == LayerIdx)
			{
				Layer[i] = UINT8_MAX;
			}
		}
		
		Layers.Add(Layer);
	}
	
	if (ReferenceSystem)
	{
		FString WorldCRS = ReferenceSystem->ProjectedCRS;
		
		// Paths to temp datasets that must be deleted before this function ends
		TArray<FString> DatasetPaths;
		
		// Warp, crop and resize each layer
		for (TArray<uint8>& Layer : Layers)
		{
			GDALDatasetRef LayerDataset =
				FGDALWarp::CreateDataset(Layer, MergedXSize, MergedYSize, ERGBFormat::Gray);
			LayerDataset->SetProjection(MergedProjection);
			LayerDataset->SetGeoTransform(MergedGeoTransform);
			
			GDALDatasetRef WarpedLayer =
				FGDALWarp::WarpDataset(LayerDataset, MergedProjection, WorldCRS);

			FString CroppedDatasetPath;
			GDALDatasetRef CroppedDataset =
				FGDALWarp::CropDataset(WarpedLayer.Get(), Bounds.TopLeft, Bounds.BottomRight, CroppedDatasetPath);
			DatasetPaths.Add(CroppedDatasetPath);

			FString ResizedDatasetPath;
			GDALDatasetRef ResizedDataset =
				FGDALWarp::ResizeDataset(CroppedDataset.Get(), RequiredResolution, ResizedDatasetPath);
			DatasetPaths.Add(ResizedDatasetPath);

			TArray<uint8> CompletedLayer;
			FGDALWarp::GetRawImage(
				ResizedDataset,
				CompletedLayer,
				RequiredResolution.X,
				RequiredResolution.Y,
				1
				);

			RawData.Add(CompletedLayer);

			Layer.Empty();
		}

		FGDALWarp::DeleteVRTDatasets(DatasetPaths);
	}
	
}

TSharedRef<FGeoTileAPI> FLandscapeImporter::GetTileAPI()
{
	TSharedPtr<FGeoTileAPI> Output;
	
	switch (EdModeConfig->LandscapeFormat)
	{
		case ELandscapeFormat::Mapbox:
			Output = MakeShared<FMapBoxTerrain>(
					EdModeConfig,
					AWorldReferenceSystem::GetWorldReferenceSystem(World)
					);
			break;
		case ELandscapeFormat::STRM:
		default:
			Output = MakeShared<FHGTTileAPI>(
				EdModeConfig,
				AWorldReferenceSystem::GetWorldReferenceSystem(World)
				);
	}

	Output->OnComplete.BindRaw(this, &FLandscapeImporter::OnTileDataLoaded);

	return Output.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE

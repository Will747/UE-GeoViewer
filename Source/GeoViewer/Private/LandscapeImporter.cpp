#include "LandscapeImporter.h"

#include "GDALWarp.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeStreamingProxy.h"
#include "Kismet/GameplayStatics.h"
#include "TileAPIs/HGTTileAPI.h"

FLandscapeImporter::FLandscapeImporter(): World(nullptr), EdModeConfig(nullptr)
{
}

void FLandscapeImporter::Initialize(UWorld* InWorld, UGeoViewerEdModeConfig* InEdModeConfig)
{
	World = InWorld;
	EdModeConfig = InEdModeConfig;

	if (EdModeConfig && World)
	{
		switch (EdModeConfig->LandscapeFormat)
		{
		case ELandscapeFormat::STRM:
		default:
			TileAPI = MakeShared<FHGTTileAPI>(
				InEdModeConfig,
				AWorldReferenceSystem::GetWorldReferenceSystem(World)
				);
		}

		TileAPI->OnComplete.BindRaw(this, &FLandscapeImporter::OnTileDataLoaded);
	}
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
		
		TileAPI->LoadTile(TileBounds);
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

void FLandscapeImporter::OnTileDataLoaded(GDALDataset* Dataset) const
{
	if (Dataset)
	{
		// Read height data from dataset
		TArray<int16> HeightDataSigned;
		GDALDatasetRef DatasetRef(Dataset);
		FGDALWarp::GetRawImage(DatasetRef, HeightDataSigned);

		const float HeightScale = UINT16_MAX / (MountEverestHeight/2);
		constexpr float SeaLevelOffset = UINT16_MAX / 2;
		
		// Convert and resize height data to uint16
		TArray<uint16> HeightData;
		HeightData.Init(0, HeightDataSigned.Num());
		for (int i = 0; i < HeightDataSigned.Num(); i++)
		{
			HeightData[i] = SeaLevelOffset + (HeightDataSigned[i] * HeightScale);
		}
		HeightDataSigned.Empty();

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
		int MinXY = 500;
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
		GDALDriver* Driver = (GDALDriver*)GDALGetDriverByName("VRT");

		for (FString DatasetPath : FilesToDelete)
		{
			Driver->Delete(TCHAR_TO_UTF8(*DatasetPath));
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load dataset"));
	}
}

void FLandscapeImporter::CreateLandscapeProxy(const TArray<uint16>& HeightData) const
{
	const ALandscape* LandscapeActor = GetLandscapeActor();
	
	ALandscapeStreamingProxy* LandscapeProxy = World->SpawnActor<ALandscapeStreamingProxy>();
	LandscapeProxy->LandscapeMaterial = EdModeConfig->LandscapeMaterial;
	LandscapeProxy->SetLandscapeGuid(FGuid::NewGuid());
	LandscapeProxy->SetActorScale3D(GetLandscapeScale());
	
	TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;

	HeightmapDataPerLayers.Add(FGuid(), HeightData);
	MaterialLayerDataPerLayer.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());

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

	LandscapeActor->GetLandscapeInfo()->FixupProxiesTransform();
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

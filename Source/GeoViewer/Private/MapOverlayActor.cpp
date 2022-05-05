#include "MapOverlayActor.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "GeoReferencingSystem.h"
#include "GeoViewer.h"
#include "LevelEditorViewport.h"
#include "OverlayTileGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "ReferenceSystems/WorldReferenceSystem.h"
#include "TileAPIs/BingMapsAPI.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AMapOverlayActor::AMapOverlayActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	USceneComponent* Component = CreateDefaultSubobject<USceneComponent>("RootComponent");
	Component->Mobility = EComponentMobility::Static;
	SetRootComponent(Component);

	bOverlayActive = false;
	
	//Material used on the decals to display the overlay
	static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/GeoViewer/M_Overlay.M_Overlay"));
	LoadingMaterial = BaseMaterial.Object;
}

AMapOverlayActor* AMapOverlayActor::GetMapOverlayActor(const UObject* WorldContext)
{
	AMapOverlayActor* Actor = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, StaticClass(), Actors);

		const int NumOfActors = Actors.Num();
		if (NumOfActors == 0)
		{
			 Actor = World->SpawnActor<AMapOverlayActor>();
		}
		else
		{
			if (NumOfActors > 1)
			{
				UE_LOG(LogGeoViewer, Warning, TEXT("%d overlay actors were found in the world. Only one should be in the world."), NumOfActors);
			}
			Actor = Cast<AMapOverlayActor>(Actors[0]);
		}
	}

	return Actor;
}

// Called every frame
void AMapOverlayActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bOverlayActive)
	{
		if (const UWorld* World = GetWorld())
		{
			const FViewportCursorLocation Cursor = GCurrentLevelEditingViewportClient->GetCursorWorldLocationFromMousePos();
			const FVector UserPosition = Cursor.GetOrigin();
			
			// Calculate the tile position/index between the origin and user
			const int TileSize = EdModeConfig->TileSize;
			const int X = UserPosition.X / TileSize;
			const int Y = UserPosition.Y / TileSize;

			// Form a string and check that tile doesn't already exist
			const FString Key =  FString::FromInt(X) + "," + FString::FromInt(Y);
			if (!Tiles.Contains(Key))
			{
				// Calculate UE coordinates for corner positions of the tile
				const float HalfTileSize = TileSize / 2;
				const FVector Center = FVector(X * TileSize, Y * TileSize, 0);
				const FVector Corner1 = FVector(HalfTileSize, HalfTileSize, 0) + Center;
				const FVector Corner2 = FVector(-HalfTileSize, -HalfTileSize, 0) + Center;

				// Make sure all the components aren't still loading before adding another tile
				const UOverlayTileComponent* NextTileComponent = GetNextComponent();
				if ((NextTileComponent && !NextTileComponent->IsLoadingTile()) || NextTileComponent == nullptr)
				{
					LoadNewTile(
					Corner1,
					Corner2,
					Key);
				}
			}
		}
	}
}

bool AMapOverlayActor::ShouldTickIfViewportsOnly() const
{
	return true;
}

bool AMapOverlayActor::GetOverlayState() const
{
	return bOverlayActive;
}

void AMapOverlayActor::Activate()
{
	bOverlayActive = true;

	// Show all tiles from web apis
	for (UOverlayTileComponent* Decal : WebDecals)
	{
		if (Decal)
		{
			Decal->SetVisibility(true);
		}
	}
}

void AMapOverlayActor::Deactivate()
{
	bOverlayActive = false;

	// Hide all tiles from web apis
	for (UOverlayTileComponent* Decal : WebDecals)
	{
		if (Decal)
		{
			Decal->SetVisibility(false);
		}
	}
}

void AMapOverlayActor::SetConfig(UGeoViewerEdModeConfig* InConfig)
{
	EdModeConfig = InConfig;
	ReloadConfig();
}

bool AMapOverlayActor::IsMissingConfig() const
{
	return !EdModeConfig.IsValid();
}

void AMapOverlayActor::ReloadConfig()
{
	if (EdModeConfig.IsValid())
	{
		for (UOverlayTileComponent* Decal : WebDecals)
		{
			if (Decal)
			{
				// Ensure all decals are properly unregistered from the actor
				Decal->DestroyComponent();
			}
		}

		// Empty and resize array for new tile amount
		WebDecals.Empty(EdModeConfig->MaxNumberOfTiles);
		WebDecals.Init(nullptr, EdModeConfig->MaxNumberOfTiles);

		Tiles.Empty();
	}
}

void AMapOverlayActor::AddOverlayTile(GDALDataset* Dataset, FString Key)
{
	UOverlayTileComponent* NextComponent = GetNextComponent();

	// Remove the generator for the tiles map as it can be destroyed once the texture is created
	const TSharedPtr<FOverlayTileGenerator> TileGenerator = Tiles[Key];
	Tiles[Key].Reset();
	
	if (NextComponent)
	{
		// If the component is still loading the previous tile the user is probably
		// moving too fast so don't add the new overlay tile for now
		if (NextComponent->IsLoadingTile())
		{
			Tiles.Remove(Key);
		}
		else
		{
			Tiles.Remove(NextComponent->Key);
			NextComponent->Key = Key;
			
			NextComponent->SetDataset(Dataset, TileGenerator);
			IncrementQueueIndex();
		}
	}
	else
	{
		// Create a new component if one does not exist as this point in the array
		UOverlayTileComponent* TileComponent = NewObject<UOverlayTileComponent>(this);
		TileComponent->CreationMethod = EComponentCreationMethod::Instance;
		TileComponent->SetDataset(Dataset, TileGenerator, LoadingMaterial);
		TileComponent->RegisterComponent();
		TileComponent->Key = Key;
		WebDecals[DecalQueueIdx] = TileComponent;
		IncrementQueueIndex();
	}
}

TSharedRef<FOverlayTileGenerator> AMapOverlayActor::LoadNewTile(const FVector Corner1, const FVector Corner2, FString Key)
{
	//Convert engine coordinates to geographic coordinates
	FGeoBounds NewTileBounds;
	AWorldReferenceSystem* WorldReferenceSystem = GetWorldReferenceSystem();
	WorldReferenceSystem->EngineToGeographic(Corner1, NewTileBounds.TopLeft);
	WorldReferenceSystem->EngineToGeographic(Corner2, NewTileBounds.BottomRight);
	
	//Request Tile
	TSharedRef<FOverlayTileGenerator> Tile = MakeShareable(new FOverlayTileGenerator());
	Tile->Key = Key;
	Tiles.Add(Key, Tile);
	Tile->GenerateTile(this, EdModeConfig, GetWorldReferenceSystem(), NewTileBounds);

	return Tile;
}

AWorldReferenceSystem* AMapOverlayActor::GetWorldReferenceSystem()
{
	if (!CachedWorldReference.IsValid())
	{
		CachedWorldReference = AWorldReferenceSystem::GetWorldReferenceSystem(GetWorld());
	}
	
	return CachedWorldReference.Get();
}

UOverlayTileComponent* AMapOverlayActor::GetNextComponent()
{
	if (DecalQueueIdx >= WebDecals.Num())
	{
		DecalQueueIdx = 0;
	}

	return WebDecals[DecalQueueIdx];
}

void AMapOverlayActor::IncrementQueueIndex()
{
	DecalQueueIdx++;

	if (DecalQueueIdx >= WebDecals.Num())
	{
		DecalQueueIdx = 0;
	}
}

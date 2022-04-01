#pragma once

#include "CoreMinimal.h"
#include "GeoViewerEdModeConfig.h"
#include "OverlayTileComponent.h"
#include "OverlayTileGenerator.h"
#include "GameFramework/Actor.h"
#include "ReferenceSystems/WorldReferenceSystem.h"
#include "MapOverlayActor.generated.h"

/**
 * Actor that generates a real world map overlay on the world using decal components.
 * This uses the GeoReferencing system to convert between real world locations and the
 * UE world positions. This will download the data from an API specified in the editor
 * mode config and is normally spawned by the editor mode.
 */
UCLASS(Transient)
class GEOVIEWER_API AMapOverlayActor : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Sets default values for this actor's properties */
	AMapOverlayActor();

	/** Gets the overlay actor if one is in the world or creates a new one */
	static AMapOverlayActor* GetMapOverlayActor(const UObject* WorldContext);

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Makes actor tick in editor as soon as placed */
	virtual bool ShouldTickIfViewportsOnly() const override;

	/** True if the overlay is active */
	bool GetOverlayState() const;
	
	/** Enables the overlay */
	void Activate();

	/** Disables the overlay */
	void Deactivate();

	/** Sets the config required for downloading tiles */
	void SetConfig(UGeoViewerEdModeConfig* InConfig);

	/** Resets all tiles when the config changes */
	void ReloadConfig();

	/** Adds a new decal to the world based on the dataset */
	void AddOverlayTile(GDALDataset* Dataset, FString Key);

	/** The default material used by decals */
	UPROPERTY(EditAnywhere)
	UMaterial* LoadingMaterial;

private:
	/** Loads a tile and adds decal at the position */
	TSharedRef<FOverlayTileGenerator> LoadNewTile(const FVector Corner1, const FVector Corner2, FString Key);

	/** Returns the reference system or create a new one if not in the world. */
	AWorldReferenceSystem* GetWorldReferenceSystem();

	UPROPERTY()
	TArray<UOverlayTileComponent*> Decals;

	UPROPERTY()
	TArray<UOverlayTileComponent*> WebDecals;

	UOverlayTileComponent* GetNextComponent();
	void IncrementQueueIndex();
	int DecalQueueIdx;

	/** If false no tile should be shown or downloaded */
	bool bOverlayActive;

	TMap<FString, TSharedPtr<FOverlayTileGenerator>> Tiles;

	TWeakObjectPtr<UGeoViewerEdModeConfig> EdModeConfig;

	/** Prevents needing to search for the reference system every time it's needed. */
	TWeakObjectPtr<AWorldReferenceSystem> CachedWorldReference;
};

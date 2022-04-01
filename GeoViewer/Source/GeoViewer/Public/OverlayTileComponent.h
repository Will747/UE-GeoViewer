#pragma once

#include "CoreMinimal.h"
#include "GDALRasterReaderWorker.h"
#include "OverlayTileGenerator.h"
#include "SmartPointers.h"
#include "Components/DecalComponent.h"
#include "OverlayTileComponent.generated.h"

/**
 * Component used for rendering a GDALDataset to the map as a decal.
 */
UCLASS(Transient)
class GEOVIEWER_API UOverlayTileComponent : public UDecalComponent
{
	GENERATED_BODY()

public:
	UOverlayTileComponent();

	// UDecalComponent Interface
	virtual void BeginDestroy() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End UDecalComponent Interface

	/**
	 * Sets a new GDALDataset for the component to render.
	 * @param Dataset The GDALDataset to display.
	 * @param InTileGenerator The generator used to create the dataset.
	 * @param InParentMaterial Optional material to replace the existing decal material.
	 */
	void SetDataset(
		GDALDataset* Dataset,
		TSharedPtr<FOverlayTileGenerator> InTileGenerator,
		UMaterialInterface* InParentMaterial = nullptr
		);

	/** True when the image is being extracted from the GDALDataset */
	bool IsLoadingTile() const;

	/** Used to link this component to a tile generator */
	FString Key;
private:
	/** Sets the texture parameter on the decal material to the pointer 'Texture' */
	void SetTextureMaterial() const;
	
	UPROPERTY()
	UTexture2D* Texture;

	TArray<uint8> RawImage;
	FGDALRasterReaderWorker* TextureWorker;

	TSharedPtr<FOverlayTileGenerator> TileGenerator;
};

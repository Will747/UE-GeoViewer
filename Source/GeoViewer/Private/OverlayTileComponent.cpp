#include "OverlayTileComponent.h"

#include "GDALWarp.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ReferenceSystems/WorldReferenceSystem.h"


// Sets default values for this component's properties
UOverlayTileComponent::UOverlayTileComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	PrimaryComponentTick.bCanEverTick = true;

}

void UOverlayTileComponent::BeginDestroy()
{
	Super::BeginDestroy();
	
	if (TextureWorker)
	{
		delete TextureWorker;
		TextureWorker = nullptr;
	}
	
	RawImage.Empty();
}

void UOverlayTileComponent::SetDataset(
	GDALDataset* Dataset,
	const TSharedPtr<FOverlayTileGenerator> InTileGenerator,
	UMaterialInterface* InParentMaterial
	)
{
	//TODO: Check the projection matches the engine projection otherwise it must be reprojected.

	// Store the generator as the Dataset may require other datasets that it holds.
	TileGenerator = InTileGenerator;

	// Remove current texture
	if (Texture)
	{
		Texture->ConditionalBeginDestroy();
		Texture = nullptr;
	}

	// Set new parent material
	if (InParentMaterial)
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(InParentMaterial, this);
		SetDecalMaterial(DynamicMaterial);
	}
	
	// Get pixel dimensions
	const int PixelNumX = Dataset->GetRasterXSize();
	const int PixelNumY = Dataset->GetRasterYSize();

	// Get transform used by the dataset
	double GeoTransform[6];
	Dataset->GetGeoTransform(GeoTransform);
	
	// Create texture from dataset
	TextureWorker = new FGDALRasterReaderWorker(Dataset, RawImage);

	// Calculate projected bounds
	AWorldReferenceSystem* ReferenceSystem = AWorldReferenceSystem::GetWorldReferenceSystem(GetWorld());
	const FVector TopCornerProj(GeoTransform[0], GeoTransform[3], 0);

	// Calculate the projected size
	const double SizeProjX = PixelNumX * GeoTransform[1];
	const double SizeProjY = PixelNumY * GeoTransform[5];

	// Calculate the bottom corner projected position
	FVector BottomCornerProj;
	BottomCornerProj.X = TopCornerProj.X + SizeProjX;
	BottomCornerProj.Y = TopCornerProj.Y + SizeProjY;

	// Convert projected corner to engine corner
	FVector TopCorner;
	ReferenceSystem->ProjectedToEngine(TopCornerProj, TopCorner);

	// Calculate size based on both corners
	FVector Size;
	ReferenceSystem->ProjectedToEngine(BottomCornerProj, Size);
	Size.X = FMath::Abs(Size.X - TopCorner.X);
	Size.Y = FMath::Abs(Size.Y - TopCorner.Y);

	// Rearrange components from Size for the Decal Size
	DecalSize.X = 90000; //Should be a large value so that it covers all heights of terrain
	DecalSize.Y = Size.X / 2;
	DecalSize.Z = Size.Y / 2;

	// Calculate engine center position
	FVector Center;
	Center.X = TopCorner.X + (Size.X / 2);
	Center.Y = TopCorner.Y + (Size.Y / 2);

	// Resize and position decal for the dataset
	SetRelativeRotation(FRotator(270, 0, 0));
	SetWorldLocation(Center);
}

bool UOverlayTileComponent::IsLoadingTile() const
{
	return !Texture && TextureWorker;
}

void UOverlayTileComponent::SetOpacity(const float Opacity) const
{
	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(DecalMaterial);
	DynamicMaterial->SetScalarParameterValue("Opacity", Opacity);
}

void UOverlayTileComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsLoadingTile())
	{
		if (TextureWorker->bDone)
		{
			TextureWorker->bDone = false;
			
			Texture = FGDALWarp::CreateTexture2D(
				this,
				RawImage,
				TextureWorker->SizeX,
				TextureWorker->SizeY
				);
			SetTextureMaterial();

			delete TextureWorker;
			TextureWorker = nullptr;
			RawImage.Empty();

			TileGenerator.Reset();
		}
	}
}

void UOverlayTileComponent::SetTextureMaterial() const
{
	// Update material with new texture
	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(DecalMaterial);
	DynamicMaterial->SetTextureParameterValue("MapTexture", Texture);
}

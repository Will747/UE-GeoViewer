#pragma once

#include "CoreMinimal.h"
#include "GeoViewerEdModeConfig.generated.h"

UENUM()
enum class EGoogleMapType : uint8
{
	Satellite,
	RoadMap,
	Terrain,
	Hybrid
};

USTRUCT()
struct FGoogleMapsOverlayConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, NonTransactional)
	int ZoomLevel = 19;

	/** Pixel size of each tile */
	UPROPERTY(EditAnywhere, NonTransactional, meta = (UIMin=300, UIMax=640))
	int TileResolution = 640;
	
	/** The type of map used */
	UPROPERTY(EditAnywhere, NonTransactional)
	EGoogleMapType Type = EGoogleMapType::Satellite;

	UPROPERTY(EditAnywhere, NonTransactional, DisplayName="Show API Key")
	bool bShowAPIKey = false;
	
	UPROPERTY(EditAnywhere, NonTransactional, DisplayName="API Key", meta = (EditConditionHides, EditCondition = "bShowAPIKey"))
	FString APIKey;
};

UENUM()
enum class EBingMapType : uint8
{
	Aerial,
	AerialWithLabels,
	Road,
	CanvasDark,
	CanvasLight,
	CanvasGray
};

USTRUCT()
struct FBingMapsOverlayConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, NonTransactional)
	int ZoomLevel = 19;

	/** Pixel size of each tile */
	UPROPERTY(EditAnywhere, NonTransactional)
	int TileResolution = 1500;

	/** The type of map used */
	UPROPERTY(EditAnywhere, NonTransactional)
	EBingMapType Type = EBingMapType::Aerial;

	UPROPERTY(EditAnywhere, NonTransactional, DisplayName="Show API Key")
	bool bShowAPIKey = false;
	
	UPROPERTY(EditAnywhere, NonTransactional, DisplayName="API Key", meta = (EditConditionHides, EditCondition = "bShowAPIKey"))
	FString APIKey;
};

UENUM()
enum class EOverlayMapSystem : uint8
{
	GoogleMaps,
	BingMaps
};

UENUM()
enum class ELandscapeFormat : uint8
{
	STRM UMETA(DisplayName = "STRM HGT Format")
};

/**
 * The config shown on the editor mode panel
 */
UCLASS(MinimalAPI)
class UGeoViewerEdModeConfig : public UObject
{
	GENERATED_BODY()

public:
	class FGeoViewerEdMode* ParentMode;
	
	/** The size in the world of one overlay component in centimeters */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Overlay", meta = (UIMin=10000, UIMax=1000000))
	int TileSize = 20000;

	/** The maximum number of decals being used at one time */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Overlay", meta = (UIMin=1, UIMax=50))
	int MaxNumberOfTiles = 9;

	//TODO: Doesn't work at the moment
	/** Opacity of the decal material where 0 is not transparent and 1 is fully transparent */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Overlay", meta = (UIMin=0, UIMax=1))
	float Opacity = 0;
	
	/** The API to get the imagery from */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Overlay")
	EOverlayMapSystem OverlaySystem;
	
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Bing API Config", meta = (ShowOnlyInnerProperties))
	FBingMapsOverlayConfig BingMaps;

	UPROPERTY(EditAnywhere, NonTransactional, Category = "Google API Config", meta = (ShowOnlyInnerProperties))
	FGoogleMapsOverlayConfig GoogleMaps;

	UPROPERTY(EditAnywhere, NonTransactional, Category = "Landscape")
	ELandscapeFormat LandscapeFormat;

	/** Number of quads in a section. This can be: 7, 15, 31, 63, 127, 255 (63 = 63x63) */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Landscape", meta = (UIMin=7, UIMax=255))
	int SectionSize = 63;
	
	/** Number of components per landscape proxy */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Landscape", meta = (UIMin=1, UIMax=1024))
	int NumberOfComponents = 64;

	/** Number of sections a component is split into. (2 = 4x4 sections) */
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Landscape", meta = (UIMin=1, UIMax=2))
	int SectionsPerComponent = 2;

	UPROPERTY(EditAnywhere, NonTransactional, Category = "Landscape")
	UMaterialInterface* LandscapeMaterial;
	
	/** Loads and Saves config to ini file */
	void Load();
	void Save();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};

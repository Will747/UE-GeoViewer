#pragma once

#include "CoreMinimal.h"
#include "GeoViewerEdMode.h"
#include "MapOverlayActor.h"
#include "Toolkits/BaseToolkit.h"

/**
 * Used for managing the UI panel of the GeoViewer editor mode.
 * Including the config and buttons to enable or disable the overlay.
 */
class FGeoViewerEdModeToolkit : public FModeToolkit
{
public:

	FGeoViewerEdModeToolkit();
	
	// FModeToolkit interface
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	// IToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FGeoViewerEdMode* GetEditorMode() const override;
	virtual TSharedPtr<SWidget> GetInlineContent() const override { return ToolkitWidget; }
	// End IToolkit interface
	
	UWorld* GetWorld();
	AMapOverlayActor* GetOverlayActor();

private:
	FText GetOverlayButtonText() const;
	FReply OnOverlayButtonPressed();

	TSharedPtr<SWidget> ToolkitWidget;
	TSharedPtr<IDetailsView> DetailsPanel;
	
	bool bOverlayActive;
};

/**
 * Slate widget for displaying current geographical & projected
 * coordinates
 */
class SGeoViewerCoordinates : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SGeoViewerCoordinates ) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<FGeoViewerEdModeToolkit> InParentToolkit);

private:
	FText GetProjectedCoordinatesText() const;
	FText GetGeoCoordinatesText() const;

	TWeakPtr<FGeoViewerEdModeToolkit> ParentToolkit;
};

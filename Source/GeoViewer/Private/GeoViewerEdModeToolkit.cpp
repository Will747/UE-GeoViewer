// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeoViewerEdModeToolkit.h"
#include "GeoViewerEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "GeoReferencingSystem.h"
#include "LevelEditorViewport.h"

#define LOCTEXT_NAMESPACE "GeoViewerEdModeUI"

FGeoViewerEdModeToolkit::FGeoViewerEdModeToolkit()
	: bOverlayActive(false)
{
}

void FGeoViewerEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	// Create details panel
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Create details view
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	if (const FGeoViewerEdMode* EdMode = GetEditorMode())
	{
		DetailsPanel->SetObject(EdMode->UISettings);
	}

	// Get overlay status from actor already in world
	if (const AMapOverlayActor* OverlayActor = GetOverlayActor())
	{
		bOverlayActive = OverlayActor->GetOverlayState();
	}

	// Create Widget
	SAssignNew(ToolkitWidget, SBorder)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot() // Overlay activate button
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.OnClicked(this, &FGeoViewerEdModeToolkit::OnOverlayButtonPressed)
					[
						SNew(STextBlock)
						.Text(this, &FGeoViewerEdModeToolkit::GetOverlayButtonText)
					]
				]
			]
			+ SVerticalBox::Slot() // Current Position Data
			.AutoHeight()
			[
				SNew(SGeoViewerCoordinates, SharedThis(this))
			]
			+ SVerticalBox::Slot() // Details panel
			[
				DetailsPanel.ToSharedRef()
			]
		];
		
	FModeToolkit::Init(InitToolkitHost);
}

FName FGeoViewerEdModeToolkit::GetToolkitFName() const
{
	return FName("GeoViewerEdMode");
}

FText FGeoViewerEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("GeoViewerEdModeToolkit", "DisplayName", "GeoViewerEdMode Tool");
}

UWorld* FGeoViewerEdModeToolkit::GetWorld() const
{
	if (const FGeoViewerEdMode* EdMode = GetEditorMode())
	{
		return EdMode->GetWorld();
	}

	return nullptr;
}

FText FGeoViewerEdModeToolkit::GetOverlayButtonText() const
{
	if (bOverlayActive)
	{
		return LOCTEXT("GeoViewerOverlayDeactivateButton", "Deactivate Overlay");
	}

	return LOCTEXT("GeoViewerOverlayActivateButton", "Activate Overlay");
}

FReply FGeoViewerEdModeToolkit::OnOverlayButtonPressed()
{
	bOverlayActive = !bOverlayActive;

	AMapOverlayActor* CurrentOverlayActor = GetOverlayActor();

	if (CurrentOverlayActor)
	{
		// Sync overlay status with overlay actor
		if (bOverlayActive)
		{
			CurrentOverlayActor->Activate();
		} else
		{
			CurrentOverlayActor->Deactivate();
		}
	}

	return FReply::Handled();
}

AMapOverlayActor* FGeoViewerEdModeToolkit::GetOverlayActor() const
{
	if (FGeoViewerEdMode* EdMode = GetEditorMode())
	{
		return EdMode->GetOverlayActor();
	}

	return nullptr;
}

FGeoViewerEdMode* FGeoViewerEdModeToolkit::GetEditorMode() const
{
	return (FGeoViewerEdMode*)GLevelEditorModeTools().GetActiveMode(FGeoViewerEdMode::EM_GeoViewerEdModeId);
}

//////////////////////////////////////////////////////////////////////////
/// SGeoViewerCoordinates

void SGeoViewerCoordinates::Construct(const FArguments& InArgs, TSharedRef<FGeoViewerEdModeToolkit> InParentToolkit)
{
	ParentToolkit = InParentToolkit;

	ChildSlot
		[
			SNew(SBorder)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("GeoViewer", "GeographicalPositionLabel", "Geographical Position:"))
				]
				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Text(this, &SGeoViewerCoordinates::GetGeoCoordinatesText)
				]
			]
		];

}

FText SGeoViewerCoordinates::GetGeoCoordinatesText() const
{
	if (ParentToolkit.IsValid())
	{
		// Find the world position of the viewport
		const FViewportCursorLocation ViewportCursor =
			GCurrentLevelEditingViewportClient->GetCursorWorldLocationFromMousePos();
		const FVector UserPosition = ViewportCursor.GetOrigin();

		// Convert the engine coordinates to geographic
		const UWorld* World = ParentToolkit.Pin()->GetWorld();
		if (World)
		{
			AWorldReferenceSystem* GeoSystem = AWorldReferenceSystem::GetWorldReferenceSystem(World);

			if (GeoSystem)
			{
				FGeographicCoordinates GeographicCoordinates;
				GeoSystem->EngineToGeographic(UserPosition, GeographicCoordinates);
				return GeographicCoordinates.ToFullText();
			}
		}
	}

	return FText();
}

#undef LOCTEXT_NAMESPACE

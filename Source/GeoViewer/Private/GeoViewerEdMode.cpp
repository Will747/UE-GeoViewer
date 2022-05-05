// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeoViewerEdMode.h"

#include "GeoViewerEdModeToolkit.h"
#include "EditorModeManager.h"

const FEditorModeID FGeoViewerEdMode::EM_GeoViewerEdModeId = TEXT("EM_GeoViewerEdMode");

FGeoViewerEdMode::FGeoViewerEdMode()
{
	UISettings = NewObject<UGeoViewerEdModeConfig>(GetTransientPackage());
	UISettings->ParentMode = this;
}

FGeoViewerEdMode::~FGeoViewerEdMode()
{
}

void FGeoViewerEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid())
	{
		Toolkit = MakeShareable(new FGeoViewerEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
	
	UISettings->Load();

	if (AMapOverlayActor* CurrentMapOverlayActor = GetOverlayActor())
	{
		CurrentMapOverlayActor->SetConfig(UISettings);
	}
}

void FGeoViewerEdMode::Exit()
{
	UISettings->Save();
	
	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

void FGeoViewerEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(UISettings);
}

bool FGeoViewerEdMode::UsesToolkits() const
{
	return true;
}

void FGeoViewerEdMode::ResetOverlay()
{
	if (AMapOverlayActor* CurrentOverlayActor = GetOverlayActor())
	{
		CurrentOverlayActor->ReloadConfig();
	}
}

AMapOverlayActor* FGeoViewerEdMode::GetOverlayActor()
{
	if (!OverlayActor.IsValid())
	{
		if (const UWorld* World = GetWorld())
		{
			OverlayActor = AMapOverlayActor::GetMapOverlayActor(World);
		}

		if (!OverlayActor.IsValid())
		{
			return nullptr;
		}
	}

	if (OverlayActor->IsMissingConfig())
	{
		OverlayActor->SetConfig(UISettings);
	}

	return OverlayActor.Get();
}





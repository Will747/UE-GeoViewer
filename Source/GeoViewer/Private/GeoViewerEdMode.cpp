// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeoViewerEdMode.h"

#include "GeoViewerEdModeToolkit.h"
#include "EditorModeManager.h"
#include "LevelEditorViewport.h"
#include "Kismet/GameplayStatics.h"
#include "Toolkits/ToolkitManager.h"

const FEditorModeID FGeoViewerEdMode::EM_GeoViewerEdModeId = TEXT("EM_GeoViewerEdMode");

FGeoViewerEdMode::FGeoViewerEdMode()
{
	UISettings = NewObject<UGeoViewerEdModeConfig>(GetTransientPackage());
	UISettings->ParentMode = this;

	LandscapeImporter = MakeUnique<FLandscapeImporter>();
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

	LandscapeImporter->Initialize(GetWorld(), UISettings);
	
	UISettings->Load();

	if (ALandscape* Landscape = GetLandscape())
	{
		UISettings->InitializeLandscapeLayers(Landscape);
	}
	
	if (AMapOverlayActor* CurrentMapOverlayActor = GetOverlayActor())
	{
		CurrentMapOverlayActor->SetConfig(UISettings);
	}
}

void FGeoViewerEdMode::Exit()
{
	UISettings->Save();

	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}
	
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

void FGeoViewerEdMode::LoadTerrainAroundUser()
{
	// Find the world position of the viewport
	const FViewportCursorLocation ViewportCursor =
		GCurrentLevelEditingViewportClient->GetCursorWorldLocationFromMousePos();
	const FVector UserPosition = ViewportCursor.GetOrigin();
	
	LandscapeImporter->LoadTile(UserPosition);
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

ALandscape* FGeoViewerEdMode::GetLandscape() const
{
	if (const UWorld* World = GetWorld())
	{
		// Search world for landscape actors
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Actors);

		if (Actors.Num() > 0)
		{
			return Cast<ALandscape>(Actors[0]);
		}
	}

	return nullptr;
}

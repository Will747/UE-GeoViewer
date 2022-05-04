// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeoViewer.h"
#include "GeoViewerEdMode.h"
#include "GeoViewerStyle.h"
#include "UnrealGDAL.h"

#define LOCTEXT_NAMESPACE "FGeoViewerModule"

void FGeoViewerModule::StartupModule()
{
	// Load Custom Slate Icons
	FGeoViewerStyle::Initialize();
	FGeoViewerStyle::ReloadTextures();

	// Add Editor Mode
	FEditorModeRegistry::Get().RegisterMode<FGeoViewerEdMode>(
		FGeoViewerEdMode::EM_GeoViewerEdModeId, 
		LOCTEXT("GeoViewerEdModeName", "Geo Viewer"), 
		FSlateIcon(FGeoViewerStyle::GetStyleSetName(), "GeoViewer.Icon", "GeoViewer.Icon.Small"),
		true
		);

	//Initialize GDAL
	FUnrealGDALModule* UnrealGDAL = FModuleManager::Get().LoadModulePtr<FUnrealGDALModule>("UnrealGDAL");
	UnrealGDAL->InitGDAL();
}

void FGeoViewerModule::ShutdownModule()
{
	FGeoViewerStyle::Shutdown();

	FEditorModeRegistry::Get().UnregisterMode(FGeoViewerEdMode::EM_GeoViewerEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGeoViewerModule, GeoViewer)
DEFINE_LOG_CATEGORY(LogGeoViewer);

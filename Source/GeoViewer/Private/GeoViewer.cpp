// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeoViewer.h"
#include "GeoViewerEdMode.h"
#include "GeoViewerSettings.h"
#include "GeoViewerStyle.h"
#include "ISettingsModule.h"

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

	// Add Editor Preferences
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Editor", "Plugins", "GeoViewer",
			LOCTEXT("SettingsDisplayName", "Geo Viewer"),
			LOCTEXT("SettingsDescription", "Configuration settings for the GeoViewer plugin."),
			GetMutableDefault<UGeoViewerSettings>()
			);
	}
	
	// Initialize GDAL
	FString GDALDataPath =
		FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Data"), TEXT("GDAL")));
	CPLSetConfigOption("GDAL_DATA", TCHAR_TO_UTF8(*GDALDataPath));
	
	GDALAllRegister();
}

void FGeoViewerModule::ShutdownModule()
{
	FGeoViewerStyle::Shutdown();
	
	FEditorModeRegistry::Get().UnregisterMode(FGeoViewerEdMode::EM_GeoViewerEdModeId);
	
	// Unregister Editor Preferences
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Editor", "Plugins", "GeoViewer");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGeoViewerModule, GeoViewer)
DEFINE_LOG_CATEGORY(LogGeoViewer);

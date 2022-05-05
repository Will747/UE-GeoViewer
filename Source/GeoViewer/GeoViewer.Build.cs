// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GeoViewer : ModuleRules
{
	public GeoViewer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Engine",
				"GeoReferencing",
				"GDAL",
				"UnrealEd"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"CoreUObject",
                "Slate",
				"SlateCore",
				"InputCore",
				"EditorFramework",
				"EditorStyle",
				"LevelEditor",
				"HTTP",
				"DesktopPlatform",
				"LandscapeEditor"
                // ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
            }
			);
	}
}

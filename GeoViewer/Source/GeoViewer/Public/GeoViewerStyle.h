#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FGeoViewerStyle
{
public:
	/** Creates and registers style instance */
	static void Initialize();

	/** Unregisters the style instance */
	static void Shutdown();

	/** Reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the plugin */
	static const ISlateStyle& Get();

	/** @return The name of the style set */
	static FName GetStyleSetName();

private:

	/** Creates the style set for this module */
	static TSharedRef<FSlateStyleSet> Create();
	
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};
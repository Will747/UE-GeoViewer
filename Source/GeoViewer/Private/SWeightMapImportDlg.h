#pragma once
#include "TileAPIS/GeoTileAPI.h"

/**
 * Widget for selecting weight map file to be used by landscape proxy.
 */
class SWeightMapImportDlg : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SWeightMapImportDlg )
		{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SWindow> InParentWindow, const FGeoBounds& InLandscapeBounds);

	/** Returns file paths of all weight maps selected by user. */
	TArray<FString>& GetFilePaths() { return Files; }
	
private:
	/** Updates files box with files */
	void RefreshFilesList();
	
	/** Returns text used by continue button */
	FText GetContinueText() const;

	/** Opens file dialog */
	FReply OnSelectWeightMapClicked();

	/** Closes this window */
	FReply OnContinueClicked() const;

	/** Saves landscape bounds to GeoJson file */
	FReply OnSaveGeoJsonClicked() const;
	
	TSharedPtr<SWindow> ParentWindow;
	FGeoBounds LandscapeBounds;

	TSharedPtr<SVerticalBox> FilesListBox;

	/* Paths to all weight maps */
	TArray<FString> Files;
};

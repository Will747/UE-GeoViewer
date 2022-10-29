#pragma once
#include "SGeoBoundsWidget.h"
#include "ReferenceSystems/WorldReferenceSystem.h"
#include "TileAPIS/GeoTileAPI.h"

/**
 *	Widget for selecting how many tiles should be loaded.
 */
class SLandscapeSizeDlg : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SLandscapeSizeDlg )
	{ }
	SLATE_END_ARGS()

	void Construct(
		const FArguments& InArgs,
		TSharedPtr<SWindow> InParentWindow,
		AWorldReferenceSystem* InReferenceSystem,
		FVector& InBottomCorner,
		const float InTileLength
		);

	int GetXSize() const;
	int GetYSize() const;
private:
	void OnXSizeChange(int NewXSize);
	void OnYSizeChange(int NewYSize);
	FReply OnContinueClicked() const;

	FGeoBounds GetLandscapeBounds() const;
	
	/** Updates LandscapeBounds based on X & YSize. */
	void UpdateBounds();
	
	TSharedPtr<SWindow> ParentWindow;
	TWeakObjectPtr<AWorldReferenceSystem> ReferenceSystem;

	/** Bottom position of bottom landscape tile. */
	FVector BottomCorner;

	/** Side length of one landscape tile. */
	float TileLength = 0;

	/** Geographical bounds including all tiles. */
	FGeoBounds LandscapeBounds;

	/** Number of landscape tiles. */
	int XSize = 1;
	int YSize = 1;
};

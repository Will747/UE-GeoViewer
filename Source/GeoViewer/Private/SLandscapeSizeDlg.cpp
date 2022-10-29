#include "SLandscapeSizeDlg.h"

#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SGridPanel.h"

#define LOCTEXT_NAMESPACE "GeoViewerLandscapeSizeDlg"

void SLandscapeSizeDlg::Construct(const FArguments& InArgs, TSharedPtr<SWindow> InParentWindow,
                                  AWorldReferenceSystem* InReferenceSystem, FVector& InBottomCorner, const float InTileLength)
{
	ParentWindow = InParentWindow;
	ReferenceSystem = InReferenceSystem;
	BottomCorner = InBottomCorner;
	TileLength = InTileLength;
	UpdateBounds();
	
	constexpr int MaxNumOfTiles = 20;
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			[
				//X and Y size inputs
				SNew(SGridPanel)
				.FillColumn(1, 1.f)

				+SGridPanel::Slot(0, 0)
				.Padding(2.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("X", "x:"))
				]

				+SGridPanel::Slot(1, 0)
				.Padding(2.f)
				[
					SNew(SSpinBox<int>)
					.Value(this, &SLandscapeSizeDlg::GetXSize)
					.OnValueChanged(this, &SLandscapeSizeDlg::OnXSizeChange)
					.MinValue(1)
					.MaxValue(MaxNumOfTiles)
				]

				+SGridPanel::Slot(0, 1)
				.Padding(2.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Y", "y:"))
				]

				+SGridPanel::Slot(1, 1)
				.Padding(2.f)
				[
					SNew(SSpinBox<int>)
					.Value(this, &SLandscapeSizeDlg::GetYSize)
					.OnValueChanged(this, &SLandscapeSizeDlg::OnYSizeChange)
					.MinValue(1)
					.MaxValue(MaxNumOfTiles)
				]
			]

			+SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)

				+SVerticalBox::Slot()
				[
					SNew(SGeoBoundsWidget)
					.Bounds(this, &SLandscapeSizeDlg::GetLandscapeBounds)
				]

				+SVerticalBox::Slot()
				.HAlign(HAlign_Right)
				.AutoHeight()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
					.Text(LOCTEXT("ContinueBtnText", "Continue"))
					.OnClicked(this, &SLandscapeSizeDlg::OnContinueClicked)
				]
			]
		]
	];
	
}

int SLandscapeSizeDlg::GetXSize() const
{
	return XSize;
}

int SLandscapeSizeDlg::GetYSize() const
{
	return YSize;
}

void SLandscapeSizeDlg::OnXSizeChange(const int NewXSize)
{
	XSize = NewXSize;
	UpdateBounds();
}

void SLandscapeSizeDlg::OnYSizeChange(const int NewYSize)
{
	YSize = NewYSize;
	UpdateBounds();
}

FGeoBounds SLandscapeSizeDlg::GetLandscapeBounds() const
{
	return LandscapeBounds;
}

FReply SLandscapeSizeDlg::OnContinueClicked() const
{
	ParentWindow->RequestDestroyWindow();

	return FReply::Handled();
}

void SLandscapeSizeDlg::UpdateBounds()
{
	FVector TopCorner = BottomCorner;
	TopCorner.X += TileLength * XSize;
	TopCorner.Y += TileLength * YSize;

	if (ReferenceSystem.IsValid())
	{
		ReferenceSystem->EngineToGeographic(TopCorner, LandscapeBounds.TopLeft);
		ReferenceSystem->EngineToGeographic(BottomCorner, LandscapeBounds.BottomRight);
	}
}

#undef LOCTEXT_NAMESPACE

#include "SGeoBoundsWidget.h"
#include "TileAPIs/GeoTileAPI.h"

#define LOCTEXT_NAMESPACE "GeoViewerGeoBoundsWidget"

void SGeoBoundsWidget::Construct(const FArguments& InArgs)
{
	BoundsAttribute = InArgs._Bounds;

	ChildSlot
	[
		SNew(SBorder)
		.Padding(5)
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LandscapeBoundsText", "Landscape Bounds"))
			]

			+SVerticalBox::Slot()
			[
				SNew(STextBlock)
				.Text(this, &SGeoBoundsWidget::GetTopLeftText)
			]
				
			+SVerticalBox::Slot()
			[
				SNew(STextBlock)
				.Text(this, &SGeoBoundsWidget::GetBottomRightText)
			]
		]
	];
}

FText SGeoBoundsWidget::GetTopLeftText() const
{
	const FGeoBounds GeoBounds = BoundsAttribute.Get();
	return FText::Format(
		LOCTEXT("LandscapeTopLeftText", "Top Left - {0}"),
		GetCoordinateText(GeoBounds.TopLeft)
		);
}

FText SGeoBoundsWidget::GetBottomRightText() const
{
	const FGeoBounds GeoBounds = BoundsAttribute.Get();
	return FText::Format(
		LOCTEXT("LandscapeBottomRightText", "Bottom Right - {0}"),
		GetCoordinateText(GeoBounds.BottomRight)
		);
}

FText SGeoBoundsWidget::GetCoordinateText(FGeographicCoordinates Coordinates) const
{
	TArray<FText> Args;
	Args.Init(FText(), 2);
	FText Altitude;
	Coordinates.ToSeparateTexts(Args[0], Args[1], Altitude);
	
	return FText::Join(LOCTEXT("CoordinateComma", ", "), Args);
}

#undef LOCTEXT_NAMESPACE

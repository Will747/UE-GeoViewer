#pragma once
#include "GeographicCoordinates.h"
#include "TileAPIS/GeoTileAPI.h"

/**
 * Widget for displaying the top and bottom coordinates of geographic bounds.
 */
class SGeoBoundsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SGeoBoundsWidget ) { }
	SLATE_ATTRIBUTE( FGeoBounds, Bounds )
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FText GetTopLeftText() const;
	FText GetBottomRightText() const;
	
	/** Converts geo coordinates to correct text format for this widget. */
	FText GetCoordinateText(FGeographicCoordinates Coordinates) const;

	TAttribute<FGeoBounds> BoundsAttribute;
};

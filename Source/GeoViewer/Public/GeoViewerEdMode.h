// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "GeoViewerEdModeConfig.h"
#include "LandscapeImporter.h"
#include "MapOverlayActor.h"

/**
 * GeoViewer editor mode. Controls the overlay actor in the world.
 */
class FGeoViewerEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_GeoViewerEdModeId;
	UGeoViewerEdModeConfig* UISettings;

	FGeoViewerEdMode();
	virtual ~FGeoViewerEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual bool UsesToolkits() const override;
	// End of FEdMode interface

	/** Loads landscape actors around the user */
	void LoadTerrainAroundUser(bool bLoadManyTiles = false) const;

	/** Resets the overlay when a change has been made to the config */
	void ResetOverlay();

	/** Updates the opacity of the overlay. */
	void UpdateOverlayOpacity();
	
	/** Gets the overlay actor from the world */
	AMapOverlayActor* GetOverlayActor();

private:
	/** Returns the current user position in the world */
	static FVector GetUserPosition();
	
	/** Gets the landscape actor in current world */
	ALandscape* GetLandscape() const;
	
	TWeakObjectPtr<AMapOverlayActor> OverlayActor;
	TUniquePtr<FLandscapeImporter> LandscapeImporter;
};

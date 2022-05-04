// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "GeoViewerEdModeConfig.h"
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

	/** Resets the overlay when a change has been made to the config */
	void ResetOverlay();
	
	/** Gets the overlay actor from the world */
	AMapOverlayActor* GetOverlayActor();

private:
	TWeakObjectPtr<AMapOverlayActor> OverlayActor;
};

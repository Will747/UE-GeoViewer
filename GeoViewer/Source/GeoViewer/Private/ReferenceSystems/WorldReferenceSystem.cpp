#include "ReferenceSystems/WorldReferenceSystem.h"
#include "Kismet/GameplayStatics.h"

AWorldReferenceSystem* AWorldReferenceSystem::GetWorldReferenceSystem(const UObject* WorldContext)
{
	AWorldReferenceSystem* Actor = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, StaticClass(), Actors);

		const int NbActors = Actors.Num();
		if (NbActors == 0)
		{
			Actor = World->SpawnActor<AWorldReferenceSystem>();
		}
		else
		{
			Actor = Cast<AWorldReferenceSystem>(Actors[0]);
		}
	}

	return Actor;
}

void AWorldReferenceSystem::ProjectedToGeographicWithEPSG(const FCartesianCoordinates& ProjectedCoordinates,
	FGeographicCoordinates& GeographicCoordinates, const uint16 ProjectedEPSG)
{
	AGeoViewerReferenceSystem* ReferenceSystem = GetReferenceSystem(ProjectedEPSG);
	ReferenceSystem->ProjectedToGeographic(ProjectedCoordinates, GeographicCoordinates);
}

void AWorldReferenceSystem::GeographicToProjectedWithEPSG(const FGeographicCoordinates& GeographicCoordinates,
	FCartesianCoordinates& ProjectedCoordinates, const uint16 ProjectedEPSG)
{
	AGeoViewerReferenceSystem* ReferenceSystem = GetReferenceSystem(ProjectedEPSG);
	ReferenceSystem->GeographicToProjected(GeographicCoordinates, ProjectedCoordinates);
}

AGeoViewerReferenceSystem* AWorldReferenceSystem::GetReferenceSystem(uint16 EPSG)
{
	// Find exising reference system with correct CRS
	if (ChildReferenceSystems.Contains(EPSG))
	{
		return Cast<AGeoViewerReferenceSystem>(ChildReferenceSystems[EPSG]->GetChildActor());
	}

	// Create new reference system for the specific CRS
	const FName EPSGDisplayName = FName(*EPSGToString(EPSG));
	UChildActorComponent* NewChildActor = NewObject<UChildActorComponent>(this, EPSGDisplayName);
	NewChildActor->SetChildActorClass(AGeoViewerReferenceSystem::StaticClass());
	NewChildActor->CreationMethod = EComponentCreationMethod::Instance;
	NewChildActor->RegisterComponent();
	ChildReferenceSystems.Add(EPSG, NewChildActor);

	// Setup the new reference system
	AGeoViewerReferenceSystem* NewReferenceSystem = Cast<AGeoViewerReferenceSystem>(NewChildActor->GetChildActor());
	NewReferenceSystem->ProjectedCRS = "EPSG:" + FString::FromInt(EPSG);
	NewReferenceSystem->SetGeographicalOrigin(GetGeographicalOrigin());

	return NewReferenceSystem;
}

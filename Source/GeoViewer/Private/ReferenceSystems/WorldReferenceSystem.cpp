#include "ReferenceSystems/WorldReferenceSystem.h"
#include "Kismet/GameplayStatics.h"

AWorldReferenceSystem::AWorldReferenceSystem()
{
	bOriginLocationInProjectedCRS = false;
	OriginLatitude = 54;
	OriginLongitude = -2;
	ProjectedCRS = "EPSG:27700";
}

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

void AWorldReferenceSystem::ProjectedToGeographicWithEPSG(const FVector& ProjectedCoordinates,
	FGeographicCoordinates& GeographicCoordinates, const uint16 ProjectedEPSG)
{
	AGeoViewerReferenceSystem* ReferenceSystem = GetReferenceSystem(ProjectedEPSG);
	ReferenceSystem->ProjectedToGeographic(ProjectedCoordinates, GeographicCoordinates);
}

void AWorldReferenceSystem::GeographicToProjectedWithEPSG(const FGeographicCoordinates& GeographicCoordinates,
	FVector& ProjectedCoordinates, const uint16 ProjectedEPSG)
{
	AGeoViewerReferenceSystem* ReferenceSystem = GetReferenceSystem(ProjectedEPSG);
	ReferenceSystem->GeographicToProjected(GeographicCoordinates, ProjectedCoordinates);
}

void AWorldReferenceSystem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Update properties on all child actors
	for (const TPair<uint16, UChildActorComponent*>& ChildActorPair : ChildReferenceSystems)
	{
		const UChildActorComponent* ChildActorComponent = ChildActorPair.Value;

		if (ChildActorComponent->GetChildActor())
		{
			AGeoViewerReferenceSystem* ChildReferencingSystem =
				Cast<AGeoViewerReferenceSystem>(ChildActorComponent->GetChildActor());

				ChildReferencingSystem->UpdateActorSettings(this, ChildActorPair.Key);
		}
	}
}

AGeoViewerReferenceSystem* AWorldReferenceSystem::GetReferenceSystem(uint16 InEPSG)
{
	// Find exising reference system with correct CRS
	if (ChildReferenceSystems.Contains(InEPSG))
	{
		return Cast<AGeoViewerReferenceSystem>(ChildReferenceSystems[InEPSG]->GetChildActor());
	}

	// Create new reference system for the specific CRS
	const FName EPSGDisplayName = FName(*EPSGToString(InEPSG));
	UChildActorComponent* NewChildActor = NewObject<UChildActorComponent>(this, EPSGDisplayName);
	NewChildActor->SetChildActorClass(AGeoViewerReferenceSystem::StaticClass());
	NewChildActor->CreationMethod = EComponentCreationMethod::Instance;
	NewChildActor->RegisterComponent();
	ChildReferenceSystems.Add(InEPSG, NewChildActor);

	// Setup the new reference system
	AGeoViewerReferenceSystem* NewReferenceSystem = Cast<AGeoViewerReferenceSystem>(NewChildActor->GetChildActor());
	NewReferenceSystem->UpdateActorSettings(this, InEPSG);

	return NewReferenceSystem;
}

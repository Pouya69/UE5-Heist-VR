// Fill out your copyright notice in the Description page of Project Settings.


#include "Grabbable.h"

#include "Environment/Core/HeistGrabComponent.h"


AGrabbable::AGrabbable()
{
	// PrimaryActorTick.bCanEverTick = true;
	
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComp"));
	SetRootComponent(BaseMeshComponent);
	
	GrabComponent = CreateDefaultSubobject<UHeistGrabComponent>(TEXT("BaseMeshGrabComp"));
	GrabComponent->SetupAttachment(BaseMeshComponent);
	GrabComponent->InitializeGrabComponent(BaseMeshComponent);
	GrabComponent->GrabTypeBase = EGrabTypeBase::FREE;
}

void AGrabbable::SetIsInFocus_Implementation(const bool bIsInFocus)
{
	TArray<UHeistGrabComponent*> GrabComponents;
	Execute_GetGrabComponents(this, GrabComponents);

	for (UHeistGrabComponent* TempGrabComponent : GrabComponents)
	{
		Cast<UMeshComponent>(TempGrabComponent->GetPrimitiveComponentAttached())->SetOverlayMaterial(bIsInFocus ? InFocusMaterial : nullptr);
	}
}

bool AGrabbable::IsRemoteGrabbable_Implementation() const
{
	return !GrabComponent->IsBeingHeld() && !GetWorldTimerManager().IsTimerActive(RemoteGrabTimerHandle);
}

bool AGrabbable::RemoteGrab_Implementation()
{
	Execute_SetIsInFocus(this, false);
	GetWorldTimerManager().SetTimer(RemoteGrabTimerHandle, 1.8f, false);
	
	return true;
}

bool AGrabbable::GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents)
{
	if (GrabComponent && GrabComponent->IsGrabComponentReady())
	{
		OutGrabComponents.Add(GrabComponent);
		return true;
	}
	return false;
}

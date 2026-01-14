// Fill out your copyright notice in the Description page of Project Settings.


#include "Grabbable.h"

#include "Core/HeistGrabComponent.h"

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

bool AGrabbable::GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents)
{
	if (GrabComponent && GrabComponent->IsGrabComponentReady())
	{
		OutGrabComponents.Add(GrabComponent);
		return true;
	}
	return false;
}

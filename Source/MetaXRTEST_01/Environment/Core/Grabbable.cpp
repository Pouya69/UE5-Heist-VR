// Fill out your copyright notice in the Description page of Project Settings.


#include "Grabbable.h"

#include "Core/HeistFunctionLibrary.h"
#include "Core/HeistGameMode.h"
#include "Core/HeistTypes.h"
#include "Environment/Core/HeistGrabComponent.h"
#include "Kismet/GameplayStatics.h"


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
	
	IsRemoteGrabbable = true;
	
	CurrentSize = EHeistSize::MEDIUM;
}

UPrimitiveComponent* AGrabbable::GetMainPrimitiveComponent() const
{
	return GrabComponent->GetPrimitiveComponentAttached();
}

EGrabTypeBase AGrabbable::GetGrabType() const
{
	return GrabComponent->GrabTypeBase;
}

void AGrabbable::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	
	const bool bActive = NewPlayerSize == CurrentSize;
	const bool bIsTiny = CurrentSize == EHeistSize::TINY;
	
	UPrimitiveComponent* MainPrimitiveComp = GetMainPrimitiveComponent();
	
	if (IsRemoteGrabbable)
	{
		MainPrimitiveComp->SetSimulatePhysics(bActive);
		
		if (bIsTiny)
		{
			MainPrimitiveComp->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		}
	}
		
	
	ToggleActivateGrabbable(bActive);
	
	switch (NewPlayerSize)
	{
		case EHeistSize::MEDIUM:
			if (bIsTiny)
			{
				// Disappear tiny stuff.
				GrabComponent->SetGrabbableVisible(false);
			}
			SetActorScale3D(StartingScale);
			SetActorLocation(GetActorLocation() * 0.001, false, nullptr, ETeleportType::TeleportPhysics);
			break;
		
		case EHeistSize::TINY:
			if (bIsTiny)
			{
				// Disappear tiny stuff.
				GrabComponent->SetGrabbableVisible(true);
			}
			// For now, everything would be visible in Grabbable
			SetActorScale3D(StartingScale * 1000.0f);
			SetActorLocation(GetActorLocation() * MEDIUM_SIZE_MULT, false, nullptr, ETeleportType::TeleportPhysics);
			break;
	}
}

void AGrabbable::ToggleActivateGrabbable(const bool bActive)
{
	bIsGrabbableActive = bActive;
	// FTimerHandle TH;
}

void AGrabbable::SetIsInFocus_Implementation(const bool bIsInFocus)
{
	TArray<UHeistGrabComponent*> GrabComponents;
	Execute_GetGrabComponents(this, GrabComponents);

	for (const UHeistGrabComponent* TempGrabComponent : GrabComponents)
	{
		Cast<UMeshComponent>(TempGrabComponent->GetPrimitiveComponentAttached())->SetOverlayMaterial(bIsInFocus ? InFocusMaterial : nullptr);
	}
}

bool AGrabbable::IsRemoteGrabbable_Implementation() const
{
	return bIsGrabbableActive && IsRemoteGrabbable && !GrabComponent->IsBeingHeld() && !GetWorldTimerManager().IsTimerActive(RemoteGrabTimerHandle);
}

bool AGrabbable::RemoteGrab_Implementation()
{
	Execute_SetIsInFocus(this, false);
	GetWorldTimerManager().ClearTimer(RemoteGrabTimerHandle);
	GetWorldTimerManager().SetTimer(RemoteGrabTimerHandle, 1.8f, false);
	
	return true;
}

bool AGrabbable::IsGrabbable_Implementation(const FName BoneHit) const
{
	return bIsGrabbableActive;
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

EHeistGrabHandState AGrabbable::GetHandAnimationType_Implementation() const
{
	return EHeistGrabHandState::DEFAULT;
}

EHeistSize AGrabbable::GetCurrentSizeOfGameObject_Implementation()
{
	return CurrentSize;
}

void AGrabbable::SetNewSizeTo_Implementation(EHeistSize NewSize)
{
	CurrentSize = NewSize;
}

void AGrabbable::OnReleased_Default(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef)
{
	GetWorldTimerManager().ClearTimer(RemoteGrabTimerHandle);
	GetWorldTimerManager().SetTimer(RemoteGrabTimerHandle, 1.8f, false);
}

void AGrabbable::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		ToggleActivateGrabbable(EHeistSize::MEDIUM == CurrentSize);
		
		if (CurrentSize == EHeistSize::TINY)
		{
			GrabComponent->SetGrabbableVisible(false);
		}
		
		GrabComponent->OnReleased.AddDynamic(this, &AGrabbable::OnReleased_Default);
		AHeistGameMode* GM = Cast<AHeistGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		GM->OnPlayerChangeSize.AddDynamic(this, &AGrabbable::OnPlayerChangeSize);
		
		StartingScale = GetActorScale3D();
	}
}



#include "HeistGrabComponent.h"
#include "MotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"


UHeistGrabComponent::UHeistGrabComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	GrabTypeBase = EGrabTypeBase::FREE;
	bSimulateOnDrop = true;
}

void UHeistGrabComponent::InitializeGrabComponent(UPrimitiveComponent* InPrimitiveComp, const bool bWasInitializedFromActor)
{
	PrimitiveComponent = InPrimitiveComp;
	
	if (bWasInitializedFromActor) return;
	
	PrimitiveComponent->SetSimulatePhysics(true);
	PrimitiveComponent->SetCollisionProfileName("VR_Grabbable");
}

UMotionControllerComponent* UHeistGrabComponent::GetCurrentMotionControllerHoldingThis()
{
	return CurrentMotionControllerHoldingThis;
}

bool UHeistGrabComponent::IsGrabComponentReady() const
{
	return PrimitiveComponent != nullptr;
}

bool UHeistGrabComponent::AttachPrimitiveCompToMotionController(UMotionControllerComponent* MotionController)
{
	if (ensureMsgf(IsGrabComponentReady(), TEXT("Grab Component is not ready on actor. FIX: Make sure InitializeGrabComponent() is being called.")))
	{
		const bool bAttached = PrimitiveComponent->AttachToComponent(MotionController, FAttachmentTransformRules::KeepWorldTransform);
		
		if (!bAttached)
		{
			UE_LOG(LogTemp, Error, TEXT("%s to %s not attached. FAILED."), *PrimitiveComponent->GetName(), *MotionController->GetName());
			return false;
		}
		
		return true;
	}
	
	return false;
}

bool UHeistGrabComponent::TryGrab(UMotionControllerComponent* MotionController, APlayerController* PlayerController)
{
	switch (GrabTypeBase)
	{
		case EGrabTypeBase::NONE:
			break;
	
		case EGrabTypeBase::FREE:
			SetPrimitiveComponentPhysicsEnabled(false);
			AttachPrimitiveCompToMotionController(MotionController);
			bIsHeld = true;
			break;
		
		case EGrabTypeBase::SNAP:
			SetPrimitiveComponentPhysicsEnabled(false);
			AttachPrimitiveCompToMotionController(MotionController);
			SnapToMotionController(MotionController);
			bIsHeld = true;
			break;
		
		case EGrabTypeBase::CUSTOM:
			bIsHeld = true;
			break;
		
		default:
			break;
	}
	
	if (!bIsHeld) return false;
	
	CurrentMotionControllerHoldingThis = MotionController;
	PlayerController->PlayHapticEffect(OnGrabHapticEffect, GetHeldByHand());
	
	UGameplayStatics::PlaySoundAtLocation(this, OnGrabSound, GetComponentLocation());
	
	// For custom, we can just use this and bind it there.
	OnGrabbed.Broadcast(this, MotionController);
	
	return true;
}

bool UHeistGrabComponent::TryRelease(UMotionControllerComponent* MotionController, APlayerController* PlayerController)
{
	switch (GrabTypeBase)
	{
		case EGrabTypeBase::NONE:
			break;
		
		case EGrabTypeBase::FREE:
			if (bSimulateOnDrop)
				SetPrimitiveComponentPhysicsEnabled(true);
			DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::SNAP:
			if (bSimulateOnDrop)
				SetPrimitiveComponentPhysicsEnabled(true);
			DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::CUSTOM:
			bIsHeld = false;
			break;
		
		default:
			break;
	}
	
	// Still haven't released.
	if (bIsHeld) return false;
	
	PlayerController->PlayHapticEffect(OnReleaseHapticEffect, GetHeldByHand());
	CurrentMotionControllerHoldingThis = nullptr;
	
	UGameplayStatics::PlaySoundAtLocation(this, OnReleaseSound, GetComponentLocation());
	
	OnReleased.Broadcast(this, MotionController);
	
	return true;
}

void UHeistGrabComponent::SetSimulateOnDrop(const bool bSimulate)
{
	if (ensureMsgf(IsGrabComponentReady(), TEXT("Grab Component is not ready on actor. FIX: Make sure InitializeGrabComponent() is being called.")))
	{
		if (PrimitiveComponent->IsAnySimulatingPhysics())
			bSimulateOnDrop = bSimulate;
	}
}

void UHeistGrabComponent::SetPrimitiveComponentPhysicsEnabled(const bool bSimulate)
{
	if (ensureMsgf(IsGrabComponentReady(), TEXT("Grab Component is not ready on actor. FIX: Make sure InitializeGrabComponent() is being called.")))
	{
		PrimitiveComponent->SetSimulatePhysics(bSimulate);
	}
}

void UHeistGrabComponent::SnapToMotionController(UMotionControllerComponent* MotionController, FVector LocationOffset,
	FRotator RotationOffset)
{
	FHitResult HitResult_Rotation;
	FHitResult HitResult_Location;
	PrimitiveComponent->SetRelativeRotation((GetRelativeRotation() + RotationOffset).GetInverse(), false, &HitResult_Rotation, ETeleportType::TeleportPhysics);
	
	const FVector Location = MotionController->GetComponentLocation() + ((GetComponentLocation() - PrimitiveComponent->GetComponentLocation()) * -1.0f);
	PrimitiveComponent->SetWorldLocation(Location, false, &HitResult_Rotation, ETeleportType::TeleportPhysics);
	
}

EControllerHand UHeistGrabComponent::GetHeldByHand() const
{
	return CurrentMotionControllerHoldingThis->MotionSource.IsEqual("LeftGrip") ? EControllerHand::Left : EControllerHand::Right;
}

bool UHeistGrabComponent::IsBeingHeld() const
{
	return bIsHeld;
}


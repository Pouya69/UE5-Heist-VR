

#include "HeistGrabComponent.h"

#include "Core/HeistInteractionInterface.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Player/HeistPlayerInterface.h"




UHeistGrabComponent::UHeistGrabComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	GrabTypeBase = EGrabTypeBase::FREE;
	bSimulateOnDrop = true;
}

void UHeistGrabComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// THIS SECTION IS ONLY FOR TWO HANDED.
	
	DetachWhenTooFarFromGrabbable();
	// GetCurrentMotionControllerHoldingThis()->PhysicsHandRef->SetRelativeLocation(GrabLocationCache);
}


void UHeistGrabComponent::InitializeGrabComponent(UPrimitiveComponent* InPrimitiveComp, const bool bWasInitializedFromActor, UHeistGrabComponent* OtherGrabComponentForTwoHanded)
{
	if (PrimitiveComponent != nullptr)
	{
		// If child classes override what GrabComponent gets attached to, we clear whatever we set previous.
		PrimitiveComponent->SetSimulatePhysics(false);
		PrimitiveComponent->SetCollisionProfileName("PhysicsActor");
	}
	
	PrimitiveComponent = InPrimitiveComp;
	OtherGrabComponent = OtherGrabComponentForTwoHanded;
	
	if (bWasInitializedFromActor) return;
	
	PrimitiveComponent->SetSimulatePhysics(true);
	SetSimulateOnDrop(true);
	PrimitiveComponent->SetCollisionProfileName("VR_Grabbable");
}

void UHeistGrabComponent::AttachHandToGrabComponent(bool bAttach,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	if (bAttach)
	{
		// Attach Physics hand to this object.
		// We snap to closest point
		MotionControllerRef->PhysicsHandRef->AttachToComponent(PrimitiveComponent, 
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false), NAME_None);
		
		const FVector Start = MotionControllerRef->PhysicsHandRef->GetSocketLocation(GetHeldByHand(MotionControllerRef) == EControllerHand::Left ? "hand_l" : "hand_r");
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredComponent(MotionControllerRef->PhysicsHandRef.Get());
		CollisionParams.AddIgnoredComponent(MotionControllerRef->GhostHandRef.Get());
		const bool bWasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, Start, FQuat::Identity,ECC_WorldDynamic, FCollisionShape::MakeSphere(2.f), CollisionParams);
		ensure(bWasHit && HitResult.GetComponent() == PrimitiveComponent);
		
		GrabLocationCache = PrimitiveComponent->GetComponentTransform().TransformVector(HitResult.ImpactPoint);
		MotionControllerRef->PhysicsHandRef->SetRelativeLocation(GrabLocationCache, false, nullptr, ETeleportType::TeleportPhysics);
		
		SetComponentTickEnabled(true);
		
		
		return;
	}
	
	SetComponentTickEnabled(false);
	// Detach from object and let it simulate.
	if (MotionControllerRef)
		MotionControllerRef->PhysicsHandRef->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void UHeistGrabComponent::TraceFingersProcedural()
{
	// Trace for accurate procedural gripping.
	switch (GetHeldByHand())
	{
	case EControllerHand::Right:
		IHeistPlayerInterface::Execute_TraceRightFinger(CurrentMotionControllerHoldingThis->GetOwner());
		break;
		
	case EControllerHand::Left:
		IHeistPlayerInterface::Execute_TraceLeftFinger(CurrentMotionControllerHoldingThis->GetOwner());
		break;
		
	default:
		break;
	}
}

UHeistMotionControllerComponent* UHeistGrabComponent::GetCurrentMotionControllerHoldingThis()
{
	return CurrentMotionControllerHoldingThis;
}

bool UHeistGrabComponent::IsGrabComponentReady() const
{
	return GrabTypeBase == EGrabTypeBase::TWO_HANDED ? (OtherGrabComponent != nullptr && PrimitiveComponent != nullptr) : PrimitiveComponent != nullptr;
}

bool UHeistGrabComponent::IsReadyToGrab() const
{
	return GrabTypeBase == EGrabTypeBase::TWO_HANDED ? OtherGrabComponent->IsBeingHeld() : true;
}

bool UHeistGrabComponent::AttachPrimitiveCompTo(USceneComponent* AttachTo)
{
	if (ensureMsgf(IsGrabComponentReady(), TEXT("Grab Component is not ready on actor. FIX: Make sure InitializeGrabComponent() is being called.")))
	{
		
		const bool bAttached = PrimitiveComponent->AttachToComponent(AttachTo, 
			FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,EAttachmentRule::KeepWorld, true));
		
		if (!bAttached)
		{
			UE_LOG(LogTemp, Error, TEXT("%s to %s not attached. FAILED."), *PrimitiveComponent->GetName(), *AttachTo->GetName());
			return false;
		}
		
		return true;
	}
	
	return false;
}

bool UHeistGrabComponent::TryGrab(UHeistMotionControllerComponent* MotionController, USceneComponent* AttachTo, APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint)
{
	if (!ensureMsgf(MotionController->IsMotionControllerReady(), TEXT("Motion Controller is not initialized. FIX: Do InitializeMotionControllerComponent() in BP.")))
		return false;
	
	if (IsBeingHeld())
	{
		// Already being held.
		// TryRelease(nullptr, PlayerController);
	}
	
	switch (GrabTypeBase)
	{
		case EGrabTypeBase::NONE:
			break;
	
		case EGrabTypeBase::FREE:
			SetPrimitiveComponentPhysicsEnabled(false);
			AttachPrimitiveCompTo(AttachTo);
			// SetPrimitiveComponentPhysicsEnabled(true);
			bIsHeld = true;
			break;
		
		case EGrabTypeBase::SNAP:
			SetPrimitiveComponentPhysicsEnabled(false);
			AttachPrimitiveCompTo(AttachTo);
			SnapTo(MotionController);
			// SetPrimitiveComponentPhysicsEnabled(true);
			bIsHeld = true;
			break;
		
		case EGrabTypeBase::CUSTOM:
			bIsHeld = true;
			break;
		
		case EGrabTypeBase::WEIGHTED_ONE_HANDED:
			if (PhysicsConstraintGrabbingThis && PhysicsConstraintGrabbingThis != HandPhysicsConstraint)
			{
				PhysicsConstraintGrabbingThis->BreakConstraint();
			}
			PhysicsConstraintGrabbingThis = HandPhysicsConstraint;
			PhysicsConstraintGrabbingThis->SetConstrainedComponents(CastChecked<USkeletalMeshComponent>(AttachTo), GetHeldByHand(MotionController) == EControllerHand::Left ? "hand_l" : "hand_r", PrimitiveComponent, NAME_None);
			// SetPrimitiveComponentPhysicsEnabled(false);
			// AttachPrimitiveCompTo(AttachTo);
			SetComponentTickEnabled(true);
			bIsHeld = true;
			break;
		
		case EGrabTypeBase::TWO_HANDED:
			bIsHeld = true;
			if (IsReadyToGrab())
			{
				AttachPrimitiveCompTo(AttachTo);
				
				// Grabbed two-handed fully.
				// We detach the hands and then attach the object to them.
				
				// AttachPrimitiveCompTo(AttachTo);
				
				// AttachHandToGrabComponent(false, MotionController);
				// OtherGrabComponent->AttachHandToGrabComponent(false, OtherGrabComponent->GetCurrentMotionControllerHoldingThis());
				
			}
			else
			{
				if (bIsHeld)
				{
					// Already held. We grab the other component.
					return OtherGrabComponent->TryGrab(MotionController, AttachTo, PlayerController, HandPhysicsConstraint);
				}
				
				PhysicsConstraintGrabbingThis = HandPhysicsConstraint;
				PhysicsConstraintGrabbingThis->SetConstrainedComponents(CastChecked<USkeletalMeshComponent>(AttachTo), GetHeldByHand(MotionController) == EControllerHand::Left ? "hand_l" : "hand_r", PrimitiveComponent, NAME_None);
				// SetPrimitiveComponentPhysicsEnabled(false);
				SetComponentTickEnabled(true);
				bIsHeld = true;
			}
			break;
		
		default:
			break;
	}
	
	if (!bIsHeld) return false;
	
	IHeistInteractionInterface::Execute_SetIsInFocus(GetOwner(), false);
	
	CurrentMotionControllerHoldingThis = MotionController;
	PlayerController->PlayHapticEffect(OnGrabHapticEffect, GetHeldByHand());
	
	UGameplayStatics::PlaySoundAtLocation(this, OnGrabSound, GetComponentLocation());
	
	if (IsReadyToGrab() && GrabTypeBase == EGrabTypeBase::TWO_HANDED)
		OnGrabbedTwoHanded.Broadcast(this, OtherGrabComponent, MotionController);
	else
		OnGrabbed.Broadcast(this, MotionController);
	
	return true;
}

bool UHeistGrabComponent::TryRelease(UHeistMotionControllerComponent* MotionController, APlayerController* PlayerController)
{
	switch (GrabTypeBase)
	{
		case EGrabTypeBase::NONE:
			break;
		
		case EGrabTypeBase::FREE:
			if (bSimulateOnDrop)
			{
				SetPrimitiveComponentPhysicsEnabled(true);
			}
			DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::SNAP:
			if (bSimulateOnDrop)
			{
				SetPrimitiveComponentPhysicsEnabled(true);
			}
			DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::WEIGHTED_ONE_HANDED:
			DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			if (PhysicsConstraintGrabbingThis)
			{
				PhysicsConstraintGrabbingThis->BreakConstraint();
				PhysicsConstraintGrabbingThis = nullptr;
			}
			SetPrimitiveComponentPhysicsEnabled(true);
			SetComponentTickEnabled(false);
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::CUSTOM:
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::TWO_HANDED:
			bIsHeld = false;
			if (PhysicsConstraintGrabbingThis)
			{
				PhysicsConstraintGrabbingThis->BreakConstraint();
				PhysicsConstraintGrabbingThis = nullptr;
			}
			else
			{
				DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			}
			SetPrimitiveComponentPhysicsEnabled(true);
			if (!OtherGrabComponent->IsBeingHeld())
			{
				OtherGrabComponent->SetComponentTickEnabled(false);
				SetComponentTickEnabled(false);
			}
			// AttachHandToGrabComponent(false, MotionController);
			// OtherGrabComponent->AttachHandToGrabComponent(false, OtherGrabComponent->GetCurrentMotionControllerHoldingThis());
			break;
		
		default:
			break;
	}
	
	// Still haven't released.
	if (bIsHeld)
	{
		return false;
	}
	
	if (CurrentMotionControllerHoldingThis)
		PlayerController->PlayHapticEffect(OnReleaseHapticEffect, GetHeldByHand(CurrentMotionControllerHoldingThis));
	CurrentMotionControllerHoldingThis = nullptr;
	
	UGameplayStatics::PlaySoundAtLocation(this, OnReleaseSound, GetComponentLocation());
	
	OnReleased.Broadcast(this, MotionController);
	
	return true;
}

void UHeistGrabComponent::SetSimulateOnDrop(const bool bSimulate)
{
	if (ensureMsgf(IsGrabComponentReady(), TEXT("Grab Component is not ready on actor. FIX: Make sure InitializeGrabComponent() is being called.")))
	{
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

UPrimitiveComponent* UHeistGrabComponent::GetPrimitiveComponentAttached() const
{
	return PrimitiveComponent;
}

void UHeistGrabComponent::SnapTo(USceneComponent* AttachTo, FVector LocationOffset,
                                 FRotator RotationOffset)
{
	FHitResult HitResult_Rotation;
	FHitResult HitResult_Location;
	PrimitiveComponent->SetRelativeRotation((GetRelativeRotation() + RotationOffset).GetInverse(), false, &HitResult_Rotation, ETeleportType::TeleportPhysics);
	
	const FVector Location = AttachTo->GetComponentLocation() + ((GetComponentLocation() - PrimitiveComponent->GetComponentLocation()) * -1.0f);
	PrimitiveComponent->SetWorldLocation(Location, false, &HitResult_Rotation, ETeleportType::TeleportPhysics);
	
}

EControllerHand UHeistGrabComponent::GetHeldByHand(UHeistMotionControllerComponent* InMotionController) const
{
	if (CurrentMotionControllerHoldingThis == nullptr)
		return InMotionController->MotionSource.IsEqual("Left") ? EControllerHand::Left : EControllerHand::Right;
	return CurrentMotionControllerHoldingThis->MotionSource.IsEqual("Left") ? EControllerHand::Left : EControllerHand::Right;
}

bool UHeistGrabComponent::IsBeingHeld() const
{
	return bIsHeld;
}

bool UHeistGrabComponent::DetachWhenTooFarFromGrabbable(const float DistanceThreshold)
{
	if (CurrentMotionControllerHoldingThis == nullptr || 
		FVector::Dist(CurrentMotionControllerHoldingThis->GhostHandRef->GetComponentLocation(), PrimitiveComponent->GetComponentLocation()) <= DistanceThreshold) return false;
	
	TryRelease(CurrentMotionControllerHoldingThis, UGameplayStatics::GetPlayerController(GetWorld(), 0));
	return true;
}


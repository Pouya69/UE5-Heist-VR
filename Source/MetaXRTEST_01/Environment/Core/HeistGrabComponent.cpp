

#include "HeistGrabComponent.h"

#include "Core/HeistInteractionInterface.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Player/HeistPlayerInterface.h"




UHeistGrabComponent::UHeistGrabComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;
	
	GrabTypeBase = EGrabTypeBase::FREE;
	bSimulateOnDrop = true;
	
	HandDetachmentDistanceThreshold = 50.0f;
}

void UHeistGrabComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// THIS SECTION IS ONLY FOR TWO HANDED.
	
	DetachWhenTooFarFromGrabbable();
	// GetCurrentMotionControllerHoldingThis()->PhysicsHandRef->SetRelativeLocation(GrabLocationCache);
}


void UHeistGrabComponent::InitializeGrabComponent(UPrimitiveComponent* InPrimitiveComp, const bool bWasInitializedFromActor)
{
	if (PrimitiveComponent != nullptr)
	{
		// If child classes override what GrabComponent gets attached to, we clear whatever we set previous.
		PrimitiveComponent->SetSimulatePhysics(false);
		if (PrimitiveComponent->GetCollisionProfileName() != "VR_Grabbable_TwoHanded")
			PrimitiveComponent->SetCollisionProfileName("PhysicsActor");
	}
	
	PrimitiveComponent = InPrimitiveComp;
	
	if (bWasInitializedFromActor) return;
	
	PrimitiveComponent->SetCanEverAffectNavigation(false);
	PrimitiveComponent->SetSimulatePhysics(true);
	SetSimulateOnDrop(true);
	if (PrimitiveComponent->GetCollisionProfileName() != "VR_Grabbable_TwoHanded")
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
	return PrimitiveComponent != nullptr;
}

bool UHeistGrabComponent::IsReadyToGrab() const
{
	// return true;
	return GrabTypeBase == EGrabTypeBase::TWO_HANDED ? (bIsHeld || bIsHeld_02) : true;
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

void UHeistGrabComponent::SetGrabbableVisible(const bool bIsGrabbableVisible)
{
	PrimitiveComponent->SetVisibility(bIsGrabbableVisible);
}

bool UHeistGrabComponent::TryGrab(UHeistMotionControllerComponent* MotionController, USceneComponent* AttachTo, APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint, const FVector& SnapGrabLocationOffset)
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
			SnapTo(MotionController, SnapGrabLocationOffset, FRotator::ZeroRotator, GetHeldByHand(MotionController) == EControllerHand::Right);
			
			// SetPrimitiveComponentPhysicsEnabled(true);
			bIsHeld = true;
			break;
		
		case EGrabTypeBase::CUSTOM:
			if (bIsHeld)
			{
				// Already grabbed.
				if (GetHeldByHand() == EControllerHand::Left)
					IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionController->GetOwner(), "hand_l", true, false);
				else
					IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionController->GetOwner(), "hand_r", true, false);
			}
			SetComponentTickEnabled(true);
			bIsHeld = true;
			break;
		
		case EGrabTypeBase::WEIGHTED_ONE_HANDED:
			// AttachPrimitiveCompTo(AttachTo);
			bIsHeld = true;
			SetComponentTickEnabled(true);
			if (PhysicsConstraintGrabbingThis_R && PhysicsConstraintGrabbingThis_R != HandPhysicsConstraint)
			{
				PhysicsConstraintGrabbingThis_R->BreakConstraint();
				PhysicsConstraintGrabbingThis_R = nullptr;
				FTimerDelegate TimerDelegate;
				
				
				TimerDelegate.BindLambda([&, HandPhysicsConstraint, AttachTo, MotionController]()
				{
					PhysicsConstraintGrabbingThis_R = HandPhysicsConstraint;
					PhysicsConstraintGrabbingThis_R->SetConstrainedComponents(CastChecked<USkeletalMeshComponent>(AttachTo), GetHeldByHand(MotionController) == EControllerHand::Left ? "hand_l" : "hand_r", PrimitiveComponent, NAME_None);
				});
				GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
			}
			else
			{
				PhysicsConstraintGrabbingThis_R = HandPhysicsConstraint;
				PhysicsConstraintGrabbingThis_R->SetConstrainedComponents(CastChecked<USkeletalMeshComponent>(AttachTo), GetHeldByHand(MotionController) == EControllerHand::Left ? "hand_l" : "hand_r", PrimitiveComponent, NAME_None);
			}
			
			// SetPrimitiveComponentPhysicsEnabled(false);
			break;
		
		case EGrabTypeBase::TWO_HANDED:
			const bool bIsRightHand = GetHeldByHand(MotionController) == EControllerHand::Right;
			if (bIsRightHand)
			{
				// First hand grabbing it.
				PhysicsConstraintGrabbingThis_R = HandPhysicsConstraint;
				if (!bIsHeld_02)
					PhysicsConstraintGrabbingThis_R->SetConstrainedComponents(CastChecked<USkeletalMeshComponent>(AttachTo), "hand_r", PrimitiveComponent, NAME_None);
				// SetPrimitiveComponentPhysicsEnabled(false);
				SetComponentTickEnabled(true);
				CurrentMotionControllerHoldingThis = MotionController;
				bIsHeld = true;
			}
			else
			{
				// First hand grabbing it.
				PhysicsConstraintGrabbingThis_L = HandPhysicsConstraint;
				if (!bIsHeld)
					PhysicsConstraintGrabbingThis_L->SetConstrainedComponents(CastChecked<USkeletalMeshComponent>(AttachTo), "hand_l", PrimitiveComponent, NAME_None);
				// SetPrimitiveComponentPhysicsEnabled(false);
				SetComponentTickEnabled(true);
				CurrentMotionControllerHoldingThis_02 = MotionController;
				bIsHeld_02 = true;
			}
			if (bIsHeld_02 && bIsHeld)
			{
				if (PhysicsConstraintGrabbingThis_R)
					PhysicsConstraintGrabbingThis_R->BreakConstraint();
				if (PhysicsConstraintGrabbingThis_L)
					PhysicsConstraintGrabbingThis_L->BreakConstraint();
				
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindLambda([&, HandPhysicsConstraint, AttachTo, MotionController]()
				{
					PhysicsConstraintGrabbingThis_R->SetConstrainedComponents(CurrentMotionControllerHoldingThis->PhysicsHandRef, "hand_r", PrimitiveComponent, NAME_None);
					PhysicsConstraintGrabbingThis_L->SetConstrainedComponents(CurrentMotionControllerHoldingThis_02->PhysicsHandRef, "hand_l", PrimitiveComponent, NAME_None);
					PhysicsConstraintGrabbingThis_L->UpdateConstraintFrames();
					PhysicsConstraintGrabbingThis_R->UpdateConstraintFrames();
				});
				
				GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
			}
			break;
	}
	
	const bool bIsTwoHanded = GrabTypeBase == EGrabTypeBase::TWO_HANDED;
	
	if (!bIsTwoHanded && !bIsHeld) return false;
	
	IHeistInteractionInterface::Execute_SetIsInFocus(GetOwner(), false);
	
	if (!bIsTwoHanded)
	{
		CurrentMotionControllerHoldingThis = MotionController;
	}
	MotionController->CurrentGrabbedComp = this;
	PlayerController->PlayHapticEffect(OnGrabHapticEffect, GetHeldByHand());
	
	UGameplayStatics::PlaySoundAtLocation(this, OnGrabSound, GetComponentLocation());
	
	// if (IsReadyToGrab() && GrabTypeBase == EGrabTypeBase::TWO_HANDED)
		// OnGrabbedTwoHanded.Broadcast(this, MotionController);
	
	// else
	OnGrabbed.Broadcast(this, MotionController);
	
	return true;
}

bool UHeistGrabComponent::TryRelease(UHeistMotionControllerComponent* MotionController, APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint)
{
	FVector SoundLocation = FVector::ZeroVector;
	
	switch (GrabTypeBase)
	{
		case EGrabTypeBase::NONE:
			break;
		
		case EGrabTypeBase::FREE:
			DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			if (bSimulateOnDrop)
			{
				SetPrimitiveComponentPhysicsEnabled(true);
				if (CurrentMotionControllerHoldingThis)
				{
					const FName CurrentHand = GetHeldByHand() == EControllerHand::Left ? "hand_l" : "hand_r";
					PrimitiveComponent->SetPhysicsLinearVelocity(MotionController->PhysicsHandRef->GetPhysicsLinearVelocity(CurrentHand));
					SoundLocation = CurrentMotionControllerHoldingThis->PhysicsHandRef->GetComponentLocation();
				}
			}
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::SNAP:
			DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			if (bSimulateOnDrop)
			{
				SetPrimitiveComponentPhysicsEnabled(true);
				
				if (CurrentMotionControllerHoldingThis)
				{
					const FName CurrentHand = GetHeldByHand() == EControllerHand::Left ? "hand_l" : "hand_r";
					PrimitiveComponent->SetPhysicsLinearVelocity(MotionController->PhysicsHandRef->GetPhysicsLinearVelocity(CurrentHand));
					SoundLocation = CurrentMotionControllerHoldingThis->PhysicsHandRef->GetComponentLocation();
				}
			}
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::WEIGHTED_ONE_HANDED:
			DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			if (PhysicsConstraintGrabbingThis_R)
			{
				PhysicsConstraintGrabbingThis_R->BreakConstraint();
				PhysicsConstraintGrabbingThis_R = nullptr;
				SoundLocation = CurrentMotionControllerHoldingThis->PhysicsHandRef->GetComponentLocation();
			}
			SetPrimitiveComponentPhysicsEnabled(true);
			SetComponentTickEnabled(false);
			bIsHeld = false;
			break;
		
		case EGrabTypeBase::CUSTOM:
			bIsHeld = false;
			SoundLocation = CurrentMotionControllerHoldingThis->PhysicsHandRef->GetComponentLocation();
			SetComponentTickEnabled(false);
			break;
		
		case EGrabTypeBase::TWO_HANDED:
			if (!MotionController)
			{
				break;
			}
			const bool bIsRightHand = GetHeldByHand(MotionController) == EControllerHand::Right;
			if (HandPhysicsConstraint)
			{
				HandPhysicsConstraint->BreakConstraint();
			}
			if (bIsRightHand)
			{
				CurrentMotionControllerHoldingThis->CurrentGrabbedComp = nullptr;
				if (PhysicsConstraintGrabbingThis_R)
				{
					SoundLocation = CurrentMotionControllerHoldingThis->PhysicsHandRef->GetComponentLocation();
					PhysicsConstraintGrabbingThis_R->BreakConstraint();
					PhysicsConstraintGrabbingThis_R = nullptr;
				}
				
				PlayerController->PlayHapticEffect(OnReleaseHapticEffect, GetHeldByHand(CurrentMotionControllerHoldingThis));
				CurrentMotionControllerHoldingThis = nullptr;
				bIsHeld = false;
			}
			else
			{
				CurrentMotionControllerHoldingThis_02->CurrentGrabbedComp = nullptr;
				if (PhysicsConstraintGrabbingThis_L)
				{
					SoundLocation = CurrentMotionControllerHoldingThis_02->PhysicsHandRef->GetComponentLocation();
					PhysicsConstraintGrabbingThis_L->BreakConstraint();
					PhysicsConstraintGrabbingThis_L = nullptr;
				}
				PlayerController->PlayHapticEffect(OnReleaseHapticEffect, GetHeldByHand(CurrentMotionControllerHoldingThis_02));
				CurrentMotionControllerHoldingThis_02 = nullptr;
				bIsHeld_02 = false;
			}
			if (!CurrentMotionControllerHoldingThis && !CurrentMotionControllerHoldingThis_02)
			{
				SetPrimitiveComponentPhysicsEnabled(true);
				SetComponentTickEnabled(false);
			}
			break;
	}
	
	// Still haven't released.
	const bool bIsTwoHanded = GrabTypeBase == EGrabTypeBase::TWO_HANDED;
	if (!bIsTwoHanded && bIsHeld)
	{
		return false;
	}
	
	if (!bIsTwoHanded)
	{
		if (CurrentMotionControllerHoldingThis)
		{
			CurrentMotionControllerHoldingThis->CurrentGrabbedComp = nullptr;
			PlayerController->PlayHapticEffect(OnReleaseHapticEffect, GetHeldByHand(CurrentMotionControllerHoldingThis));
		}
		CurrentMotionControllerHoldingThis = nullptr;
	}
	
	
	UGameplayStatics::PlaySoundAtLocation(this, OnReleaseSound, SoundLocation);
	
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
                                 FRotator RotationOffset, const bool bIsRightHand)
{
	PrimitiveComponent->SetRelativeRotation((GetRelativeRotation() + RotationOffset).GetInverse(), false, nullptr, ETeleportType::TeleportPhysics);
	
	const FVector Location = PrimitiveComponent->GetComponentLocation() + LocationOffset;
	PrimitiveComponent->SetWorldLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&, AttachTo, bIsRightHand]()
	{
		if (bIsRightHand)
			IHeistPlayerInterface::Execute_TraceRightFinger(AttachTo->GetOwner());
		else
			IHeistPlayerInterface::Execute_TraceLeftFinger(AttachTo->GetOwner());
	});
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
}

EControllerHand UHeistGrabComponent::GetHeldByHand(UHeistMotionControllerComponent* InMotionController) const
{
	if (InMotionController)
		return InMotionController->MotionSource.IsEqual("Left") ? EControllerHand::Left : EControllerHand::Right;
	else if (CurrentMotionControllerHoldingThis)
		return CurrentMotionControllerHoldingThis->MotionSource.IsEqual("Left") ? EControllerHand::Left : EControllerHand::Right;
	return EControllerHand::AnyHand;
}

bool UHeistGrabComponent::IsBeingHeld() const
{
	return GrabTypeBase == EGrabTypeBase::TWO_HANDED ? bIsHeld && bIsHeld_02 : bIsHeld;
}

bool UHeistGrabComponent::DetachWhenTooFarFromGrabbable()
{
	if (CurrentMotionControllerHoldingThis == nullptr || 
		FVector::Dist(CurrentMotionControllerHoldingThis->GetComponentLocation(), CurrentMotionControllerHoldingThis->PhysicsHandRef->GetComponentLocation()) <= HandDetachmentDistanceThreshold) return false;
	
	TryRelease(CurrentMotionControllerHoldingThis, UGameplayStatics::GetPlayerController(GetWorld(), 0), PhysicsConstraintGrabbingThis_R);
	return true;
}


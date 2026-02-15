// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistPlayerMainComponent.h"

#include "HeistMotionControllerComponent.h"
#include "HeistPlayerState.h"
#include "Camera/CameraComponent.h"
#include "Core/HeistInteractionInterface.h"
#include "Core/HeistTypes.h"
#include "Environment/Core/HeistGrabComponent.h"
#include "Inventory_Objects/HeistPistol.h"
#include "Kismet/GameplayStatics.h"


UHeistPlayerMainComponent::UHeistPlayerMainComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	RemoteGrabRange = 500.0f;
	RemoteGrabRadiusCheck = 20.0f;
	MinDotProductRemoteGrabThreshold = 0.2f;
	MinDotProductCameraRemoteGrabThreshold = 0.7f;
	MinForceThresholdVectorLength = 3.0f;
	RemoteGrabForceAddition = FVector(0.0f, 0.0f, 300.0f);
	
	RemoteGrabDistanceRange = FVector2D(20.0f, 500.0f);
	RemoteGrabForceRange = FVector2D(150.0f, 800.0f);
	
}

void UHeistPlayerMainComponent::InitializePlayerComponent(USkeletalMeshComponent* InRightGhostHandRef,
                                                          USkeletalMeshComponent* InRightPhysicsHandRef, USkeletalMeshComponent* InLeftGhostHandRef,
                                                          USkeletalMeshComponent* InLeftPhysicsHandRef,
                                                          UHeistMotionControllerComponent* InLeftMotionControllerRef, UHeistMotionControllerComponent* InRightMotionControllerRef,
                                                          UCameraComponent* InCameraComponent, AHeistPistol* InHeistPistol)
{
	RightGhostHandRef = InRightGhostHandRef;
	RightPhysicsHandRef = InRightPhysicsHandRef;
	
	PistolAttachedToHand = InHeistPistol;
	RightPhysicsHandRef->IgnoreActorWhenMoving(PistolAttachedToHand, true);
	PistolAttachedToHand->GetPistolSkeletalMeshComponent()->IgnoreActorWhenMoving(GetOwner(), true);
	PistolAttachedToHand->InitializeBulletPools();
	TogglePistolEnabled(false);
	
	RightMotionControllerRef = InRightMotionControllerRef;
	LeftGhostHandRef = InLeftGhostHandRef;
	LeftPhysicsHandRef = InLeftPhysicsHandRef;
	LeftMotionControllerRef = InLeftMotionControllerRef;
	
	CameraComponentRef = InCameraComponent;
	
	AHeistPlayerState* PlayerStateRef = Cast<AHeistPlayerState>(UGameplayStatics::GetPlayerState(GetWorld(), 0));
	PlayerStateRef->HeistPlayerMainComponentRef = this;
}

void UHeistPlayerMainComponent::RemoteGrabRight()
{
	FHitResult RightHitResult;
	
	const FVector RightHandLocation = RightPhysicsHandRef->GetComponentLocation();
	const FVector RightEndLocation = RightHandLocation + RightPhysicsHandRef->GetRightVector() * RemoteGrabRange;
	
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(RemoteGrabRadiusCheck);
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	
	const bool bDidRightHit = GetWorld()->SweepSingleByChannel(RightHitResult, RightHandLocation, RightEndLocation, FQuat::Identity, GRAB_CHANNEL, CollisionShape, Params);
	
	if (bDidRightHit && IHeistInteractionInterface::Execute_IsRemoteGrabbable(RightHitResult.GetActor()))
	{
		if (CurrentGrabInFocus_R && RightHitResult.GetComponent() != CurrentGrabInFocus_R)
			IHeistInteractionInterface::Execute_SetIsInFocus(CurrentGrabInFocus_R->GetOwner(), false);
		
		CurrentGrabInFocus_R = RightHitResult.GetComponent();
		IHeistInteractionInterface::Execute_SetIsInFocus(RightHitResult.GetActor(), true);
		
		const FVector CurrentHandMovementDirectionWithLength = RightHandLocation - RightHandPreviousLocation;
		
		const FVector CastDirectionOpposite = (RightHandLocation - RightEndLocation).GetSafeNormal();
		
		const float DotBetweenVelocityAndTrace = FVector::DotProduct(CastDirectionOpposite, CurrentHandMovementDirectionWithLength.GetSafeNormal());
		const float Length = CurrentHandMovementDirectionWithLength.Length();
		
		const float DotBetweenCameraForwardAndTrace = FVector::DotProduct(-CameraComponentRef->GetForwardVector(), CastDirectionOpposite);
		
		if (DotBetweenVelocityAndTrace >= MinDotProductRemoteGrabThreshold && Length >= MinForceThresholdVectorLength && DotBetweenCameraForwardAndTrace >= MinDotProductCameraRemoteGrabThreshold)
		{
			const FVector CompLoc = CurrentGrabInFocus_R->GetComponentLocation();
			const float ForceRelativeToDistance = RemoteGrabForceRange.X + ((RemoteGrabForceRange.Y - RemoteGrabForceRange.X) / (RemoteGrabDistanceRange.Y - RemoteGrabDistanceRange.X)) * (FVector::Dist(RightHandLocation, CompLoc) - RemoteGrabDistanceRange.X);
			
			const FVector FinalImpulseDirection = (RightHandLocation - CompLoc).GetSafeNormal() * ForceRelativeToDistance + RemoteGrabForceAddition;
			CurrentGrabInFocus_R->AddImpulse(FinalImpulseDirection, NAME_None, true);
			IHeistInteractionInterface::Execute_RemoteGrab(CurrentGrabInFocus_R->GetOwner());
		}
	}
	else
	{
		if (CurrentGrabInFocus_R)
		{
			IHeistInteractionInterface::Execute_SetIsInFocus(CurrentGrabInFocus_R->GetOwner(), false);
			CurrentGrabInFocus_R = nullptr;
		}
	}
	
	RightHandPreviousLocation = RightHandLocation;
}

void UHeistPlayerMainComponent::RemoteGrabLeft()
{
	FHitResult LeftHitResult;
	
	const FVector LeftHandLocation = LeftPhysicsHandRef->GetComponentLocation();
	const FVector LeftEndLocation = LeftHandLocation + LeftPhysicsHandRef->GetRightVector() * RemoteGrabRange;
	
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(RemoteGrabRadiusCheck);
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	
	const bool bDidLeftHit = GetWorld()->SweepSingleByChannel(LeftHitResult, LeftHandLocation, LeftEndLocation, FQuat::Identity, GRAB_CHANNEL, CollisionShape, Params);
	
	if (bDidLeftHit && IHeistInteractionInterface::Execute_IsRemoteGrabbable(LeftHitResult.GetActor()))
	{
		if (CurrentGrabInFocus_L && LeftHitResult.GetComponent() != CurrentGrabInFocus_L)
			IHeistInteractionInterface::Execute_SetIsInFocus(CurrentGrabInFocus_L->GetOwner(), false);
		
		CurrentGrabInFocus_L = LeftHitResult.GetComponent();
		IHeistInteractionInterface::Execute_SetIsInFocus(LeftHitResult.GetActor(), true);
		
		const FVector CurrentHandMovementDirectionWithLength = LeftHandLocation - LeftHandPreviousLocation;
		const FVector CastDirectionOpposite = (LeftHandLocation - LeftEndLocation).GetSafeNormal();
		
		const float DotBetweenVelocityAndTrace = FVector::DotProduct(CastDirectionOpposite, CurrentHandMovementDirectionWithLength.GetSafeNormal());
		const float Length = CurrentHandMovementDirectionWithLength.Length();
		
		const float DotBetweenCameraForwardAndTrace = FVector::DotProduct(-CameraComponentRef->GetForwardVector(), CastDirectionOpposite);
		
		if (DotBetweenVelocityAndTrace >= MinDotProductRemoteGrabThreshold && Length >= MinForceThresholdVectorLength && DotBetweenCameraForwardAndTrace >= MinDotProductCameraRemoteGrabThreshold)
		{
			const FVector CompLoc = CurrentGrabInFocus_L->GetComponentLocation();
			const float ForceRelativeToDistance = RemoteGrabForceRange.X + ((RemoteGrabForceRange.Y - RemoteGrabForceRange.X) / (RemoteGrabDistanceRange.Y - RemoteGrabDistanceRange.X)) * (FVector::Dist(LeftHandLocation, CompLoc) - RemoteGrabDistanceRange.X);
			
			const FVector FinalImpulseDirection = (LeftHandLocation - CompLoc).GetSafeNormal() * ForceRelativeToDistance + RemoteGrabForceAddition;
			CurrentGrabInFocus_L->AddImpulse(FinalImpulseDirection, NAME_None, true);
			IHeistInteractionInterface::Execute_RemoteGrab(CurrentGrabInFocus_L->GetOwner());
		}
	}
	else
	{
		if (CurrentGrabInFocus_L)
		{
			IHeistInteractionInterface::Execute_SetIsInFocus(CurrentGrabInFocus_L->GetOwner(), false);
			CurrentGrabInFocus_L = nullptr;
		}
	}

	LeftHandPreviousLocation = LeftHandLocation;
}

void UHeistPlayerMainComponent::CustomGrab_Tick(const float& DeltaTime)
{
	
	if (LeftMotionControllerRef->CurrentGrabbedComp && LeftMotionControllerRef->CurrentGrabbedComp->GrabTypeBase == EGrabTypeBase::CUSTOM)
	{
		// Left grabbed custom Tick logic.
		IHeistInteractionInterface::Execute_Custom_Tick(LeftMotionControllerRef->CurrentGrabbedComp->GetOwner(), DeltaTime, LeftMotionControllerRef->CurrentGrabbedComp);
	}
	
	if (RightMotionControllerRef->CurrentGrabbedComp && RightMotionControllerRef->CurrentGrabbedComp->GrabTypeBase == EGrabTypeBase::CUSTOM)
	{
		// Right grabbed custom Tick logic.
		IHeistInteractionInterface::Execute_Custom_Tick(RightMotionControllerRef->CurrentGrabbedComp->GetOwner(), DeltaTime, RightMotionControllerRef->CurrentGrabbedComp);
	}
}

void UHeistPlayerMainComponent::Custom_PistolTick(const float& Alpha)
{
	if (!PistolAttachedToHand || PistolAttachedToHand->bIsPistolEnabled)
		return;
	PistolAttachedToHand->CustomPistolTick(Alpha);
}

void UHeistPlayerMainComponent::TogglePistolEnabled(const bool bEnabled)
{
	if (bEnabled)
	{
		PistolAttachedToHand->AttachToComponent(RightPhysicsHandRef, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true));
	}
	else
	{
		PistolAttachedToHand->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	
	PistolAttachedToHand->TogglePistolEnabled(bEnabled);
}

void UHeistPlayerMainComponent::PickedUpPistol()
{
	TogglePistolEnabled(true);
}

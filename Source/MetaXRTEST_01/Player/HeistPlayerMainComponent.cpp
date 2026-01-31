// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistPlayerMainComponent.h"

#include "Core/HeistInteractionInterface.h"
#include "Core/HeistTypes.h"


UHeistPlayerMainComponent::UHeistPlayerMainComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	RemoteGrabRange = 500.0f;
	RemoteGrabRadiusCheck = 20.0f;
	MinDotProductRemoteGrabThreshold = 0.2f;
	MinForceThresholdVectorLength = 3.0f;
	RemoteGrabForceAddition = FVector(0.0f, 0.0f, 300.0f);
	
	RemoteGrabDistanceRange = FVector2D(20.0f, 500.0f);
	RemoteGrabForceRange = FVector2D(150.0f, 800.0f);
	
}


void UHeistPlayerMainComponent::InitializePlayerComponent(USkeletalMeshComponent* InRightGhostHandRef,
	USkeletalMeshComponent* InRightPhysicsHandRef, USkeletalMeshComponent* InLeftGhostHandRef,
	USkeletalMeshComponent* InLeftPhysicsHandRef)
{
	RightGhostHandRef = InRightGhostHandRef;
	RightPhysicsHandRef = InRightPhysicsHandRef;
	LeftGhostHandRef = InLeftGhostHandRef;
	LeftPhysicsHandRef = InLeftPhysicsHandRef;
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
		
		const float DotBetweenVelocityAndTrace = FVector::DotProduct((RightHandLocation - RightEndLocation).GetSafeNormal(), CurrentHandMovementDirectionWithLength.GetSafeNormal());
		const float Length = CurrentHandMovementDirectionWithLength.Length();
		
		if (DotBetweenVelocityAndTrace >= MinDotProductRemoteGrabThreshold && Length >= MinForceThresholdVectorLength)
		{
			const FVector CompLoc = CurrentGrabInFocus_R->GetComponentLocation();
			const float ForceRelativeToDistance = (FVector::Dist(RightHandLocation, CompLoc) - RemoteGrabDistanceRange.X) / (RemoteGrabForceRange.X - RemoteGrabDistanceRange.X) * (RemoteGrabForceRange.Y - RemoteGrabForceRange.X) + RemoteGrabDistanceRange.Y;
			
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
		
		const float DotBetweenVelocityAndTrace = FVector::DotProduct((LeftHandLocation - LeftEndLocation).GetSafeNormal(), CurrentHandMovementDirectionWithLength.GetSafeNormal());
		const float Length = CurrentHandMovementDirectionWithLength.Length();
		if (DotBetweenVelocityAndTrace >= MinDotProductRemoteGrabThreshold && Length >= MinForceThresholdVectorLength)
		{
			const FVector CompLoc = CurrentGrabInFocus_L->GetComponentLocation();
			const float ForceRelativeToDistance = (FVector::Dist(LeftHandLocation, CompLoc) - RemoteGrabDistanceRange.X) / (RemoteGrabForceRange.X - RemoteGrabDistanceRange.X) * (RemoteGrabForceRange.Y - RemoteGrabForceRange.X) + RemoteGrabDistanceRange.Y;
			
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

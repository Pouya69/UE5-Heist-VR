// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistPlayerMainComponent.h"

#include "HeistMotionControllerComponent.h"
#include "HeistPlayerInterface.h"
#include "HeistPlayerState.h"
#include "Camera/CameraComponent.h"
#include "Core/HeistFunctionLibrary.h"
#include "Core/HeistGameMode.h"
#include "Core/HeistInteractionInterface.h"
#include "Core/HeistTypes.h"
#include "Environment/Core/HeistGrabComponent.h"
#include "Inventory_Objects/HeistPistol.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"


UHeistPlayerMainComponent::UHeistPlayerMainComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	RemoteGrabRange = 500.0f;
	RemoteGrabRadiusCheck = 20.0f;
	MinDotProductRemoteGrabThreshold = 0.2f;
	MinDotProductCameraRemoteGrabThreshold = 0.7f;
	MinForceThresholdVectorLength = 3.0f;
	LengthThresholdForAllowingRemoteGrab = 0.1f;
	RemoteGrabForceAddition = FVector(0.0f, 0.0f, 300.0f);
	
	RemoteGrabDistanceRange = FVector2D(20.0f, 500.0f);
	RemoteGrabForceRange = FVector2D(150.0f, 800.0f);
	
	bCanRemoteGrab_R = false;
	bCanRemoteGrab_L = false;
	
}

void UHeistPlayerMainComponent::InitializePlayerComponent(EHeistSize InCurrentSize, const float InPlayerRadius, const float InPlayerCapsuleHalfHeight, USkeletalMeshComponent* InRightGhostHandRef,
                                                          USkeletalMeshComponent* InRightPhysicsHandRef, USkeletalMeshComponent* InLeftGhostHandRef,
                                                          USkeletalMeshComponent* InLeftPhysicsHandRef, UPhysicsConstraintComponent* InRightHandPhysicsConstraint, UPhysicsConstraintComponent* InLeftHandPhysicsConstraint,
                                                          UHeistMotionControllerComponent* InLeftMotionControllerRef, UHeistMotionControllerComponent* InRightMotionControllerRef,
                                                          UCameraComponent* InCameraComponent, AHeistPistol* InHeistPistol)
{
	CurrentSize = InCurrentSize;
	PlayerCapsuleHalfHeight = InPlayerCapsuleHalfHeight;
	PlayerRadius = InPlayerRadius;
	
	RightGhostHandRef = InRightGhostHandRef;
	RightPhysicsHandRef = InRightPhysicsHandRef;
	RightHandPhysicsConstraint = InRightHandPhysicsConstraint;
	LeftHandPhysicsConstraint = InLeftHandPhysicsConstraint;
	
	PistolAttachedToHand = InHeistPistol;
	InHeistPistol->InitializePistol(RightPhysicsHandRef, true);
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
	
	const FVector CurrentHandMovementDirectionWithLength = RightHandLocation - RightHandPreviousLocation;
	const float Length = CurrentHandMovementDirectionWithLength.Length();
	if (!bCanRemoteGrab_R && Length < LengthThresholdForAllowingRemoteGrab)
	{
		bCanRemoteGrab_R = true;
	}
	
	if (!bCanRemoteGrab_R) return;
	
	const bool bDidRightHit = GetWorld()->SweepSingleByChannel(RightHitResult, RightHandLocation, RightEndLocation, FQuat::Identity, GRAB_CHANNEL, CollisionShape, Params);
	
	if (bDidRightHit && IHeistInteractionInterface::Execute_IsRemoteGrabbable(RightHitResult.GetActor()))
	{
		if (CurrentGrabInFocus_R && RightHitResult.GetComponent() != CurrentGrabInFocus_R)
			IHeistInteractionInterface::Execute_SetIsInFocus(CurrentGrabInFocus_R->GetOwner(), false);
		
		CurrentGrabInFocus_R = RightHitResult.GetComponent();
		IHeistInteractionInterface::Execute_SetIsInFocus(RightHitResult.GetActor(), true);
		

		
		const FVector CastDirectionOpposite = (RightHandLocation - RightEndLocation).GetSafeNormal();
		
		const float DotBetweenVelocityAndTrace = FVector::DotProduct(CastDirectionOpposite, CurrentHandMovementDirectionWithLength.GetSafeNormal());
		
		const float DotBetweenCameraForwardAndTrace = FVector::DotProduct(-CameraComponentRef->GetForwardVector(), CastDirectionOpposite);
		
		if (DotBetweenVelocityAndTrace >= MinDotProductRemoteGrabThreshold && Length >= MinForceThresholdVectorLength && DotBetweenCameraForwardAndTrace >= MinDotProductCameraRemoteGrabThreshold)
		{
			const FVector CompLoc = CurrentGrabInFocus_R->GetComponentLocation();
			const float ForceRelativeToDistance = RemoteGrabForceRange.X + ((RemoteGrabForceRange.Y - RemoteGrabForceRange.X) / (RemoteGrabDistanceRange.Y - RemoteGrabDistanceRange.X)) * (FVector::Dist(RightHandLocation, CompLoc) - RemoteGrabDistanceRange.X);
			
			const FVector FinalImpulseDirection = (RightHandLocation - CompLoc).GetSafeNormal() * ForceRelativeToDistance + RemoteGrabForceAddition;
			CurrentGrabInFocus_R->AddImpulse(FinalImpulseDirection, NAME_None, true);
			IHeistInteractionInterface::Execute_RemoteGrab(CurrentGrabInFocus_R->GetOwner());
			
			bCanRemoteGrab_R = false;
		}
	}
	else
	{
		if (CurrentGrabInFocus_R)
		{
			bCanRemoteGrab_R = false;
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
	
	const FVector CurrentHandMovementDirectionWithLength = LeftHandLocation - LeftHandPreviousLocation;
	const float Length = CurrentHandMovementDirectionWithLength.Length();
	if (!bCanRemoteGrab_L && Length < LengthThresholdForAllowingRemoteGrab)
	{
		bCanRemoteGrab_L = true;
	}
	
	if (!bCanRemoteGrab_L) return;
	
	const bool bDidLeftHit = GetWorld()->SweepSingleByChannel(LeftHitResult, LeftHandLocation, LeftEndLocation, FQuat::Identity, GRAB_CHANNEL, CollisionShape, Params);
	
	if (bDidLeftHit && IHeistInteractionInterface::Execute_IsRemoteGrabbable(LeftHitResult.GetActor()))
	{
		if (CurrentGrabInFocus_L && LeftHitResult.GetComponent() != CurrentGrabInFocus_L)
			IHeistInteractionInterface::Execute_SetIsInFocus(CurrentGrabInFocus_L->GetOwner(), false);
		
		CurrentGrabInFocus_L = LeftHitResult.GetComponent();
		IHeistInteractionInterface::Execute_SetIsInFocus(LeftHitResult.GetActor(), true);
		
		const FVector CastDirectionOpposite = (LeftHandLocation - LeftEndLocation).GetSafeNormal();
		
		const float DotBetweenVelocityAndTrace = FVector::DotProduct(CastDirectionOpposite, CurrentHandMovementDirectionWithLength.GetSafeNormal());
		
		const float DotBetweenCameraForwardAndTrace = FVector::DotProduct(-CameraComponentRef->GetForwardVector(), CastDirectionOpposite);
		
		if (DotBetweenVelocityAndTrace >= MinDotProductRemoteGrabThreshold && Length >= MinForceThresholdVectorLength && DotBetweenCameraForwardAndTrace >= MinDotProductCameraRemoteGrabThreshold)
		{
			const FVector CompLoc = CurrentGrabInFocus_L->GetComponentLocation();
			const float ForceRelativeToDistance = RemoteGrabForceRange.X + ((RemoteGrabForceRange.Y - RemoteGrabForceRange.X) / (RemoteGrabDistanceRange.Y - RemoteGrabDistanceRange.X)) * (FVector::Dist(LeftHandLocation, CompLoc) - RemoteGrabDistanceRange.X);
			
			const FVector FinalImpulseDirection = (LeftHandLocation - CompLoc).GetSafeNormal() * ForceRelativeToDistance + RemoteGrabForceAddition;
			CurrentGrabInFocus_L->AddImpulse(FinalImpulseDirection, NAME_None, true);
			IHeistInteractionInterface::Execute_RemoteGrab(CurrentGrabInFocus_L->GetOwner());
			
			bCanRemoteGrab_L = false;
		}
	}
	else
	{
		if (CurrentGrabInFocus_L)
		{
			bCanRemoteGrab_L = false;
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
		PistolAttachedToHand->GetPistolSkeletalMeshComponent()->SetSimulatePhysics(false);
		PistolAttachedToHand->AttachToComponent(RightPhysicsHandRef, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true), "weapon_r_socket");
	}
	else
	{
		PistolAttachedToHand->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	
	PistolAttachedToHand->TogglePistolEnabled(bEnabled);
}

bool UHeistPlayerMainComponent::ChangeSizeTo(EHeistSize NewSize, const FVector NewLocation)
{
	AActor* OwnerActor = GetOwner();
	
	IHeistPlayerInterface::Execute_SetupBothHandsBonePhysicsAndWeightRightHand_CPP(OwnerActor, false, true);
	// RightPhysicsHandRef->SetSimulatePhysics(false);
	// LeftPhysicsHandRef->SetSimulatePhysics(false);
	
	// const FVector HandScale = UHeistFunctionLibrary::GetNewSizeOfComponent(RightPhysicsHandRef, NewSize, false);
	
	// RightPhysicsHandRef->SetRelativeScale3D(HandScale);
	// LeftPhysicsHandRef->SetRelativeScale3D(HandScale);
	
	// RightPhysicsHandRef->SetBoundsScale(RightPhysicsHandRef->BoundsScale * UHeistFunctionLibrary::GetSizeMultiplierBasedOnType(CurrentSize));
	// LeftPhysicsHandRef->SetBoundsScale(LeftPhysicsHandRef->BoundsScale * UHeistFunctionLibrary::GetSizeMultiplierBasedOnType(CurrentSize));
	
	// RightPhysicsHandRef->scale
	// RightPhysicsHandRef->SetAllMassScale(ScaleMultDifference);
	// LeftPhysicsHandRef->SetAllMassScale(ScaleMultDifference);

	FVector CameraLocation = CameraComponentRef->GetRelativeLocation();
	CameraLocation.Z = 0.0f;  // Just like the Move event.
	
	const FRotator YawOnlyOwner = FRotator(0.0f, OwnerActor->GetActorRotation().Yaw, 0.0f);
	CameraLocation = YawOnlyOwner.RotateVector(CameraLocation);
	
	FVector FinalTeleportLocation;
	if (NewLocation.IsNearlyZero())
	{
		const FVector Start = CameraComponentRef->GetComponentLocation();
		const FVector End = Start - FVector(0.0f, 0.0f, PlayerCapsuleHalfHeight);
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(OwnerActor);
		CollisionParams.AddIgnoredComponent(RightPhysicsHandRef.Get());
		CollisionParams.AddIgnoredComponent(LeftPhysicsHandRef.Get());
		
		const bool bWasHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_WorldStatic, CollisionParams);
		
		if (!bWasHit)
			HitResult.ImpactPoint = End;
		FinalTeleportLocation = HitResult.ImpactPoint - CameraLocation;
	}
	else
	{
		FinalTeleportLocation = NewLocation - CameraLocation;
	}
	
	// bool bResult = UHeistFunctionLibrary::ChangeSizeTo(OwnerActor, NewSize, FinalTeleportLocation, YawOnlyOwner);
	// OwnerActor->TeleportTo(FinalTeleportLocation, YawOnlyOwner);
	
	
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UHeistPlayerMainComponent::PlayerChangedSize, 0.2f, false);
	
	AHeistGameMode* GM = GetWorld()->GetAuthGameMode<AHeistGameMode>();
	GM->ChangePlayerSize(NewSize);
	
	return true;
}

void UHeistPlayerMainComponent::PlayerChangedSize()
{
	IHeistPlayerInterface::Execute_SetupBothHandsBonePhysicsAndWeightRightHand_CPP(GetOwner(), true, true);
	// RightHandPhysicsConstraint->SetLinearPositionTarget()
	//RightHandPhysicsConstraint->UpdateConstraintFrames();
	//LeftHandPhysicsConstraint->UpdateConstraintFrames();
}

void UHeistPlayerMainComponent::PickedUpPistol()
{
	AHeistPlayerState* PlayerState = Cast<AHeistPlayerState>(UGameplayStatics::GetPlayerState(GetWorld(), 0));
	PlayerState->PickedUpPistol();
	
	TogglePistolEnabled(true);
}

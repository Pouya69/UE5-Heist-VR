// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistSkeletalGrabComponent.h"


UHeistSkeletalGrabComponent::UHeistSkeletalGrabComponent()
{
	
}


void UHeistSkeletalGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UHeistSkeletalGrabComponent::IsGrabbableBasedOnBoneHit(const FName BoneHit) const
{
	if (AcceptableBonesToGrab.IsEmpty()) return true;
	
	return BoneHit != NAME_None && AcceptableBonesToGrab.Contains(BoneHit);
}

bool UHeistSkeletalGrabComponent::TryGrab(UHeistMotionControllerComponent* MotionController, USceneComponent* AttachTo,
                                          APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint, const FVector& SnapGrabLocationOffse)
{
	return Super::TryGrab(MotionController, AttachTo, PlayerController, HandPhysicsConstraint, SnapGrabLocationOffse);
}

bool UHeistSkeletalGrabComponent::TryRelease(UHeistMotionControllerComponent* MotionController,
	APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint)
{
	
	return Super::TryRelease(MotionController, PlayerController, HandPhysicsConstraint);
}


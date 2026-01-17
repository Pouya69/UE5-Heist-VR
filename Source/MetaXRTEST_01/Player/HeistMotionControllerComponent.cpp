// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistMotionControllerComponent.h"


// Sets default values for this component's properties
UHeistMotionControllerComponent::UHeistMotionControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bTickEvenWhenPaused = true;

	PlayerIndex = 0;
	MotionSource = IMotionController::LeftHandSourceId;
	bDisableLowLatencyUpdate = false;
	bHasAuthority = false;
	bAutoActivate = true;

	// ensure InitializeComponent() gets called
	bWantsInitializeComponent = true;
}


// Called when the game starts
void UHeistMotionControllerComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UHeistMotionControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ConstrainTickGrab(DeltaTime, FQuat::Identity);
}

void UHeistMotionControllerComponent::InitializeMotionControllerComponent(USkeletalMeshComponent* InPhysicsHand, USkeletalMeshComponent* InGhostHand)
{
	PhysicsHandRef = InPhysicsHand;
	GhostHandRef = InGhostHand;
}

bool UHeistMotionControllerComponent::IsMotionControllerReady() const
{
	return PhysicsHandRef != nullptr && GhostHandRef != nullptr;
}

void UHeistMotionControllerComponent::ConstrainTickGrab(float DeltaTime, FQuat GrabRotation, FVector GrabLocation, bool bIsConstrained)
{
	if (IsActive())
	{
		FVector Position = GetRelativeTransform().GetTranslation();
		FRotator Orientation = GetRelativeTransform().GetRotation().Rotator();
		float WorldToMeters = GetWorld() ? GetWorld()->GetWorldSettings()->WorldToMeters : 100.0f;
		
		if (bIsConstrained)
		{
			// Constraints are here.
			Position = GrabLocation;
			Orientation = GrabRotation.Rotator();
			SetWorldLocationAndRotation(Position, Orientation);
			bTracked = true;
		}
		else
		{
			const bool bNewTrackedState = PollControllerState_GameThread(Position, Orientation, bProvidedLinearVelocity, LinearVelocity, bProvidedAngularVelocity, AngularVelocityAsAxisAndLength, bProvidedLinearAcceleration, LinearAcceleration, WorldToMeters);
			if (bNewTrackedState)
			{
				// Only update the location and rotation if we are tracking because we want the controller to stay in place rather than pop to 0,0,0.  
				// Note we do update the velocity and acceleration values even if untracked because we won't see any change in the position this frame.
				// This means that for brief tracking dropouts position and orientation should behave somewhat gracefully even without interpolation, but velocity/acceleration will show snaps to zero.
				SetRelativeLocationAndRotation(Position, Orientation);
			}
			// if controller tracking just kicked in or we haven't started rendering in the (possibly present) 
			// visualization component.
			if (!bTracked && bNewTrackedState)
			{
				OnActivateVisualizationComponent.Broadcast(true);
			}

			bTracked = bNewTrackedState;
		}
		

		
	}
}

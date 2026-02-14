// Fill out your copyright notice in the Description page of Project Settings.


#include "SizeChangeMachine.h"

#include "Components/SplineComponent.h"
#include "Core/HeistGrabComponent.h"
#include "Core/HeistTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"


static TAutoConsoleVariable<bool> CVarSizeMachineProgressTest(TEXT("game.sizeMachine.progressTest"), false, TEXT("If true, you can play with the spline value slider in the instance."), ECVF_Cheat);

ASizeChangeMachine::ASizeChangeMachine()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	Machine_SK_Component = CreateDefaultSubobject<USkeletalMeshComponent>("Machine_SK_Comp");
	SetRootComponent(Machine_SK_Component);
	
	BaseMeshComponent->DestroyComponent();
	
	GrabComponent->SetupAttachment(Machine_SK_Component, "hand_l");
	GrabComponent->InitializeGrabComponent(Machine_SK_Component);
	GrabComponent->SetSimulateOnDrop(false);
	GrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	
	SecondGrabComponent = CreateDefaultSubobject<UHeistGrabComponent>("SecondGrabComp");
	
	SecondGrabComponent->SetupAttachment(Machine_SK_Component, "hand_r");
	SecondGrabComponent->InitializeGrabComponent(Machine_SK_Component);
	SecondGrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	
	LeftHandMovementSplineComponent = CreateDefaultSubobject<USplineComponent>("LeftHandMovementSplineComp");
	LeftHandMovementSplineComponent->SetupAttachment(Machine_SK_Component);
	
	RightHandMovementSplineComponent = CreateDefaultSubobject<USplineComponent>("RightHandMovementSplineComp");
	RightHandMovementSplineComponent->SetupAttachment(Machine_SK_Component);
	
	Max_Y_Distance_FromGrabComps = 60.0f;
	
	SplineResetSpeed = 0.6f;
	
	IsRemoteGrabbable = false;
	
	AcceptableBonesToGrab.Add(TEXT("hand_r"));
	AcceptableBonesToGrab.Add(TEXT("hand_l"));
	AcceptableBonesToGrab.Add(TEXT("join4_l"));
	AcceptableBonesToGrab.Add(TEXT("join4_r"));
}

bool ASizeChangeMachine::IsGrabbableBasedOnBoneHit(const FName BoneHit) const
{
	if (AcceptableBonesToGrab.IsEmpty()) return true;
	
	return BoneHit != NAME_None && AcceptableBonesToGrab.Contains(BoneHit);
}


bool ASizeChangeMachine::IsGrabbable_Implementation(const FName BoneHit) const
{
	return IsGrabbableBasedOnBoneHit(BoneHit);
}

void ASizeChangeMachine::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		GrabComponent->OnGrabbed.AddDynamic(this, &ASizeChangeMachine::OnHandleGrabbed);
		SecondGrabComponent->OnGrabbed.AddDynamic(this, &ASizeChangeMachine::OnHandleGrabbed);
	
		GrabComponent->OnReleased.AddDynamic(this, &ASizeChangeMachine::OnHandleReleased);
		SecondGrabComponent->OnReleased.AddDynamic(this, &ASizeChangeMachine::OnHandleReleased);
	
		if (!CVarSizeMachineProgressTest.GetValueOnGameThread())
		{
			LeftHandFinalLocation = LeftHandMovementSplineComponent->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World, true);
			RightHandFinalLocation = RightHandMovementSplineComponent->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World, true);
		}
	}	
}

bool ASizeChangeMachine::IsReadyToPressButtons() const
{
	return GrabComponent->IsBeingHeld() && SecondGrabComponent->IsBeingHeld();
}

bool ASizeChangeMachine::IsReadyToUseSizeMachine() const
{
	return IsReadyToPressButtons() && bIsLeftHandReady && bIsRightHandReady;
}

bool ASizeChangeMachine::ChangePlayerSize()
{
	const bool bSuccess = IHeistPlayerInterface::Execute_ChangeSize(UGameplayStatics::GetPlayerPawn(this, 0), MachinePlayerSizeChange);
	
	return bSuccess;
}


void ASizeChangeMachine::OnHandleGrabbed(UHeistGrabComponent* GrabbedComponent,
                                         UHeistMotionControllerComponent* MotionControllerRef)
{
	if (GrabbedComponent == GrabComponent)
	{
		// First handle grabbed
	}
	else if (GrabbedComponent == SecondGrabComponent)
	{
		// Second Handle grabbed
	}
	
	if (IsReadyToPressButtons())
	{
		// Both handles are grabbed.
	}
	
	SetActorTickEnabled(true);
}

void ASizeChangeMachine::OnHandleReleased(UHeistGrabComponent* ReleasedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	if (ReleasedComponent == GrabComponent)
	{
		// First handle released
	}
	else if (ReleasedComponent == SecondGrabComponent)
	{
		// Second Handle released
	}
	
	if (!GrabComponent->IsBeingHeld() && !SecondGrabComponent->IsBeingHeld())
	{
		// SetActorTickEnabled(false);
	}
}

void ASizeChangeMachine::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	if (IsReadyToUseSizeMachine())
	{
		Execute_Interact(this);
		return;
	}
	
	if (GrabComponent->IsBeingHeld())
	{
		UHeistMotionControllerComponent* ControllerRef = GrabComponent->GetCurrentMotionControllerHoldingThis();
		const FTransform GrabTransform = GrabComponent->GetComponentTransform();
		FTransform ControllerTransformRelativeToGrabComp = ControllerRef->GhostHandRef->GetComponentTransform().GetRelativeTransform(GrabTransform);
		
		LeftSplineNormalizeProgress = FMath::Clamp((ControllerTransformRelativeToGrabComp.GetTranslation().Y - GrabTransform.GetTranslation().Y) / Max_Y_Distance_FromGrabComps, 0.0f, 1.0f);
		LeftHandFinalLocation = LeftHandMovementSplineComponent->GetLocationAtTime(LeftSplineNormalizeProgress, ESplineCoordinateSpace::World, true);
		
		bIsLeftHandReady = FMath::IsNearlyEqual(LeftSplineNormalizeProgress, 1.0f);
		
		ControllerTransformRelativeToGrabComp.SetTranslation(LeftHandFinalLocation + GrabOffset);
		ControllerTransformRelativeToGrabComp.SetRotation(FQuat::MakeFromEuler(GrabOffsetRotation));
		
		ControllerRef->PhysicsHandRef->SetWorldTransform(ControllerTransformRelativeToGrabComp);
		
	}
	
	if (SecondGrabComponent->IsBeingHeld())
	{
		const FTransform GrabTransform = SecondGrabComponent->GetComponentTransform();
		UHeistMotionControllerComponent* ControllerRef = SecondGrabComponent->GetCurrentMotionControllerHoldingThis();
		FTransform ControllerTransformRelativeToGrabComp = ControllerRef->GhostHandRef->GetComponentTransform().GetRelativeTransform(GrabTransform);
		
		RightSplineNormalizeProgress = FMath::Clamp((ControllerTransformRelativeToGrabComp.GetTranslation().X - GrabTransform.GetTranslation().X) / Max_Y_Distance_FromGrabComps, 0.0f, 1.0f);
		RightHandFinalLocation = RightHandMovementSplineComponent->GetLocationAtTime(RightSplineNormalizeProgress, ESplineCoordinateSpace::World, true);
		
		bIsRightHandReady = FMath::IsNearlyEqual(RightSplineNormalizeProgress, 1.0f);
		
		ControllerTransformRelativeToGrabComp.SetTranslation(RightHandFinalLocation + GrabOffset);
		ControllerTransformRelativeToGrabComp.SetRotation(FQuat::MakeFromEuler(GrabOffsetRotation));
		
		ControllerRef->PhysicsHandRef->SetWorldTransform(ControllerTransformRelativeToGrabComp);
	}
}

void ASizeChangeMachine::Interact_Implementation()
{
	APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(this, 0);
	GrabComponent->GetCurrentMotionControllerHoldingThis()->CurrentGrabbedComp = nullptr;
	SecondGrabComponent->GetCurrentMotionControllerHoldingThis()->CurrentGrabbedComp = nullptr;
	GrabComponent->TryRelease(GrabComponent->GetCurrentMotionControllerHoldingThis(), PlayerControllerRef);
	SecondGrabComponent->TryRelease(SecondGrabComponent->GetCurrentMotionControllerHoldingThis(), PlayerControllerRef);
	
	LeftSplineNormalizeProgress = 0.0f;
	RightSplineNormalizeProgress = 0.0f;
	
	LeftHandFinalLocation = LeftHandMovementSplineComponent->GetLocationAtTime(LeftSplineNormalizeProgress, ESplineCoordinateSpace::World, true);
	RightHandFinalLocation = RightHandMovementSplineComponent->GetLocationAtTime(RightSplineNormalizeProgress, ESplineCoordinateSpace::World, true);
	
	SetActorTickEnabled(false);
	
	ChangePlayerSize();
}

void ASizeChangeMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Interp back to initial if not grabbed.
	
	const bool bIsTestingProgressSpline = CVarSizeMachineProgressTest.GetValueOnGameThread();
	
	const bool bIsRightGrabbed = SecondGrabComponent->IsBeingHeld();
	const bool bIsLeftGrabbed = GrabComponent->IsBeingHeld();
	
	if (!bIsRightGrabbed)
	{
		RightSplineNormalizeProgress = FMath::FInterpConstantTo(RightSplineNormalizeProgress, 0.0f, DeltaTime, SplineResetSpeed);
		RightHandFinalLocation = RightHandMovementSplineComponent->GetLocationAtTime(RightSplineNormalizeProgress, ESplineCoordinateSpace::World, true);
	}
	if (!bIsLeftGrabbed)
	{
		LeftSplineNormalizeProgress = FMath::FInterpConstantTo(LeftSplineNormalizeProgress, 0.0f, DeltaTime, SplineResetSpeed);
		LeftHandFinalLocation = LeftHandMovementSplineComponent->GetLocationAtTime(LeftSplineNormalizeProgress, ESplineCoordinateSpace::World, true);
	}
	
	if (!bIsTestingProgressSpline && !bIsRightGrabbed && !bIsLeftGrabbed && FMath::IsNearlyZero(RightSplineNormalizeProgress) && FMath::IsNearlyZero(LeftSplineNormalizeProgress))
	{
		UE_LOG(LogTemp, Log, TEXT("Size Machine disabling..."));
		SetActorTickEnabled(false);
		return;
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistWheel.h"

#include "Core/HeistGrabComponent.h"
#include "Core/HeistTypes.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"


static TAutoConsoleVariable<bool> CVarWheelTest(TEXT("game.lever.testWheel"), false, TEXT("Enable test wheel for turning manually in editor and testing interaction functionality."), ECVF_Cheat);

AHeistWheel::AHeistWheel()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bShouldGoBackToInitialPositionWhenNotHeld = true;
	
	IsRemoteGrabbable = false;
	
	TargetFullRotationRoll = 500.0f;
	InitialOffRotationRoll = 0.0f;
	
	BaseMeshComponent->SetSimulatePhysics(false);
	
	GrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	GrabComponent->SetSimulateOnDrop(false);
	
	ProgressResetSpeed = 60.0f;
}

float AHeistWheel::GetProgressNormalized() const
{
	return (BaseMeshComponent->GetComponentTransform().GetRotation().Rotator().Roll - InitialOffRotationRoll) / (TargetFullRotationRoll - InitialOffRotationRoll);
}

void AHeistWheel::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentRotationRoll = BaseMeshComponent->GetComponentTransform().GetRotation().Rotator().Roll;
}

void AHeistWheel::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	UHeistMotionControllerComponent* MotionControllerRef = GrabComponent->GetCurrentMotionControllerHoldingThis();
	ensure(MotionControllerRef);
	
	const FTransform ControllerTransform = MotionControllerRef->GhostHandRef->GetComponentTransform();
	FTransform BaseTransform = BaseMeshComponent->GetComponentTransform();
	
	FRotator CurrentRotationBeforeApply = BaseTransform.GetRotation().Rotator();
	CurrentRotationBeforeApply.Pitch = 0.0f;
	CurrentRotationBeforeApply.Yaw = 0.0f;
	CurrentRotationBeforeApply.Roll = CurrentRotationRoll;
	
	BaseTransform.SetRotation(CurrentRotationBeforeApply.Quaternion());
	
	FTransform ControllerTransformRelativeToLeverBase = ControllerTransform.GetRelativeTransform(BaseTransform);
	
	FVector ControllerLocationRelative = ControllerTransformRelativeToLeverBase.GetTranslation();
	ControllerLocationRelative.Z = 0;
	ControllerLocationRelative.Y = 0;
	
	ControllerLocationRelative.Normalize();
	
	const float RotationValue = FMath::Clamp(ControllerLocationRelative.Rotation().Pitch, InitialOffRotationRoll, TargetFullRotationRoll);
	CurrentRotationBeforeApply.Roll = RotationValue;
	
	
	ControllerTransformRelativeToLeverBase.SetTranslation(HandLocationOffset);
	ControllerTransformRelativeToLeverBase.SetRotation((CurrentRotationBeforeApply + HandRotationOffset).Quaternion());
	
	ControllerTransformRelativeToLeverBase = ControllerTransformRelativeToLeverBase * BaseTransform;
	
	BaseMeshComponent->SetRelativeRotation(CurrentRotationBeforeApply, false, nullptr, ETeleportType::TeleportPhysics);
	
	MotionControllerRef->PhysicsHandRef->SetWorldLocationAndRotation(ControllerTransformRelativeToLeverBase.GetTranslation(), ControllerTransformRelativeToLeverBase.GetRotation(), false, nullptr, ETeleportType::ResetPhysics);
}

void AHeistWheel::Interact_Implementation()
{
	IHeistInteractionInterface::Execute_Interact(LinkedActor);
	
	Execute_SetIsInteractable(this, false);
}

bool AHeistWheel::GetIsInteractable_Implementation() const
{
	return bIsWheelInteractable;
}

void AHeistWheel::SetIsInteractable_Implementation(const bool bIsInteractable)
{
	bIsWheelInteractable = bIsInteractable;
}

void AHeistWheel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const bool bIsInteractable = Execute_GetIsInteractable(this);
	const bool bIsBeingHeld = GrabComponent->IsBeingHeld();

	const float ProgressNormalized = GetProgressNormalized();
	
		
	BaseMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, CurrentRotationRoll));
	
	switch (LeverInteractionType)
	{
	case EHeistObjectInteractionType::CONTINUOUS_ONLY:
		Execute_SetAmount(LinkedActor, ProgressNormalized);
		break;
		
	case EHeistObjectInteractionType::TRIGGER_ON_INTERACTION_ONLY:
		if (bIsInteractable)
		{
			if (FMath::IsNearlyEqual(ProgressNormalized, 1.0f, 0.01f))
			{
				// At the interaction point. Happens once until we go back to top again.
				// Interact is when it is at the bottom. For the top, we have LeverAtStartingPoint().
				Execute_Interact(this);
				return;
			}
		}
		break;
		
	case EHeistObjectInteractionType::TRIGGER_ON_INTERACTION_AND_CONTINUOUS:
		if (bIsInteractable)
		{
			if (FMath::IsNearlyEqual(ProgressNormalized, 1.0f, 0.01f))
			{
				// At the interaction point. Happens once until we go back to top again.
				// Interact is when it is at the bottom. For the top, we have LeverAtStartingPoint().
				Execute_Interact(this);
				return;
			}
		}
		Execute_SetAmount(LinkedActor, ProgressNormalized);
		break;
		
	}
	
	if (FMath::IsNearlyZero(ProgressNormalized))
	{
		// At the starting point.
		if (!bIsInteractable)
		{
			Execute_SetIsInteractable(this, true);
		}
		
		WheelAtStartingPoint();
		
		if (!bIsBeingHeld && !CVarWheelTest.GetValueOnGameThread())
		{
			SetActorTickEnabled(false);
			GrabComponent->SetComponentTickEnabled(false);
		}
		
		return;
	}
	
	if (bShouldGoBackToInitialPositionWhenNotHeld && !bIsBeingHeld)
	{
		CurrentRotationRoll = FMath::FInterpConstantTo(CurrentRotationRoll, InitialOffRotationRoll, DeltaTime, ProgressResetSpeed);
	}
}

void AHeistWheel::OnWheelGrabbed(UHeistGrabComponent* GrabbedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	if (GrabComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", false);
	else
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", false);
}

void AHeistWheel::OnWheelReleased(UHeistGrabComponent* ReleasedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	if (GrabComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", true);
	else
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", true);
}


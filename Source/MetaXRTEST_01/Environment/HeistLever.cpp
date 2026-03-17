// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistLever.h"

#include "Core/HeistGrabComponent.h"
#include "Core/HeistTypes.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"


static TAutoConsoleVariable<bool> CVarLeverTest(TEXT("game.lever.testLever"), false, TEXT("Enable test lever for turning manually in editor and testing interaction functionality."), ECVF_Cheat);

AHeistLever::AHeistLever()
{
	PrimaryActorTick.bCanEverTick = true;
	
	BaseMeshComponent->SetCollisionProfileName("WorldDynamic");
	BaseMeshComponent->SetSimulatePhysics(false);
	
	LeverHandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverHandleMeshComp"));
	LeverHandleMeshComponent->SetupAttachment(BaseMeshComponent);
	LeverHandleMeshComponent->SetCollisionProfileName("VR_Grabbable");
	
	GrabComponent->SetupAttachment(LeverHandleMeshComponent);
	GrabComponent->InitializeGrabComponent(LeverHandleMeshComponent, true);
	GrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	GrabComponent->SetSimulateOnDrop(false);
	
	InitialOffRotationPitch = 20.0f;
	TargetFullRotationPitch = 60.0f;
	
	ProgressResetSpeed = 30.0f;
	
	LeverInteractionType = EHeistObjectInteractionType::TRIGGER_ON_INTERACTION_ONLY;
	
	bIsRemoteGrabbable = false;
	
	bIsLeverInteractable = true;
	
	HandLocationOffset_FromLeverForward = 100.0f;
	
	bCanChangeSize = false;
	
	bIsSetAmount1or2 = false;
}

void AHeistLever::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		/*
		if (InitialOffRotationPitch > TargetFullRotationPitch)
		{
			const float NewInitial = InitialOffRotationPitch;
			InitialOffRotationPitch = TargetFullRotationPitch;
			TargetFullRotationPitch = NewInitial;
		}
		*/
		
		GrabComponent->OnGrabbed.AddDynamic(this, &AHeistLever::OnLeverGrabbed);
		GrabComponent->OnReleased.AddDynamic(this, &AHeistLever::OnLeverReleased);
		
		const bool bActive = CurrentSize == EHeistSize::MEDIUM;
		if (!LinkedActor)
		{
			BaseMeshComponent->SetSimulatePhysics(bActive);
		}
		BaseMeshComponent->SetVisibility(bActive, true);
	}
}

float AHeistLever::GetProgressNormalized() const
{
	return (LeverHandleMeshComponent->GetRelativeTransform().Rotator().Pitch - InitialOffRotationPitch) / (TargetFullRotationPitch - InitialOffRotationPitch);
}

void AHeistLever::ToggleLeverEnabled(const bool bEnabled, const bool bReset)
{
	if (bReset)
		LeverHandleMeshComponent->SetRelativeRotation(FRotator(InitialOffRotationPitch, 0.0f, 0.0f), false);
	if (bEnabled)
	{
		// BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		BaseMeshComponent->SetVisibility(true, true);
		BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		LeverLightStatus(false);
	}
	else
	{
		// BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BaseMeshComponent->SetVisibility(false, true);
		BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AHeistLever::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	Super::OnPlayerChangeSize(NewPlayerSize);
	
	const bool bActive = CurrentSize == NewPlayerSize;
	if (!LinkedActor)
	{
		BaseMeshComponent->SetSimulatePhysics(bActive);
	}
	if (!bIsConnectedToAnotherActor)
		BaseMeshComponent->SetVisibility(bActive, true);
}

bool AHeistLever::GetIsInteractable_Implementation() const
{
	return bIsLeverInteractable;
}

void AHeistLever::SetIsInteractable_Implementation(const bool bIsInteractable)
{
	bIsLeverInteractable = bIsInteractable;
}

void AHeistLever::Interact_Implementation()
{
	IHeistInteractionInterface::Execute_Interact(LinkedActor);
	
	LeverLightStatus(true);
	Execute_SetIsInteractable(this, false);
	
}

void AHeistLever::OnLeverGrabbed(UHeistGrabComponent* GrabbedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	SetActorTickEnabled(true);
	GrabComponent->SetComponentTickEnabled(true);
	
	if (GrabComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
	{
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", false, false);
	}
	else
	{
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", false, false);
	}
}

void AHeistLever::OnLeverReleased(UHeistGrabComponent* ReleasedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	if (!bShouldGoBackToInitialPositionWhenNotHeld)
	{
		// Because it will never go back to initial, we will just disable tick here instead.
		SetActorTickEnabled(false);
		GrabComponent->SetComponentTickEnabled(false);
	}
	
			
	if (GrabComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", true, false);
	else
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", true, false);
}

void AHeistLever::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	UHeistMotionControllerComponent* MotionControllerRef = GrabComponent->GetCurrentMotionControllerHoldingThis();
	
	const FTransform ControllerTransform = MotionControllerRef->GetComponentTransform();
	const FTransform BaseTransform = BaseMeshComponent->GetComponentTransform();
	
	FTransform ControllerTransformRelativeToLeverBase = ControllerTransform.GetRelativeTransform(BaseTransform);
	
	const float RotationValueBeforeClamp = ControllerTransformRelativeToLeverBase.GetTranslation().Rotation().Pitch;
	const float RotationValue = TargetFullRotationPitch < InitialOffRotationPitch ? FMath::Clamp(RotationValueBeforeClamp, TargetFullRotationPitch, InitialOffRotationPitch)
	: FMath::Clamp(RotationValueBeforeClamp, InitialOffRotationPitch, TargetFullRotationPitch);
	const FRotator FinalRotation = FRotator(RotationValue, 0, 0);
	
	LeverHandleMeshComponent->SetRelativeRotation(FinalRotation);
	
	const FTransform LeverTransform = LeverHandleMeshComponent->GetComponentTransform();
	
	ControllerTransformRelativeToLeverBase = ControllerTransform.GetRelativeTransform(LeverTransform);
	
	FVector HandLocation = ControllerTransformRelativeToLeverBase.GetTranslation();
	HandLocation.Y = 0.0f;
	HandLocation.Z = 0.0f;
	
		
	ControllerTransformRelativeToLeverBase.SetRotation(HandRotationOffset.Quaternion());
	ControllerTransformRelativeToLeverBase.SetTranslation(HandLocation + HandLocationOffset);
	
	ControllerTransformRelativeToLeverBase = ControllerTransformRelativeToLeverBase * LeverTransform;
	
	MotionControllerRef->PhysicsHandRef->SetWorldLocationAndRotation(LeverTransform.GetTranslation() + (LeverHandleMeshComponent->GetForwardVector() * HandLocationOffset_FromLeverForward)
		, ControllerTransformRelativeToLeverBase.GetRotation().Rotator(), false, nullptr, ETeleportType::ResetPhysics);
}

void AHeistLever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bIsInteractable = Execute_GetIsInteractable(this);
	const bool bIsBeingHeld = GrabComponent->IsBeingHeld();
	
	const float ProgressNormalized = GetProgressNormalized();
	
	// UE_LOG(LogTemp, Log, TEXT("%f"), ProgressNormalized);
	
	switch (LeverInteractionType)
	{
		case EHeistObjectInteractionType::CONTINUOUS_ONLY:
			if (bIsSetAmount1or2)
			{
				Execute_SetAmount(LinkedActor, ProgressNormalized);
			}
			else
			{
				Execute_SetAmount_02(LinkedActor, ProgressNormalized);
			}
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
			if (bIsSetAmount1or2)
			{
				Execute_SetAmount(LinkedActor, ProgressNormalized);
			}
			else
			{
				Execute_SetAmount_02(LinkedActor, ProgressNormalized);
			}
			break;
		
	}
	
	if (FMath::IsNearlyZero(ProgressNormalized))
	{
		// At the starting point.
		if (!bIsInteractable)
		{
			Execute_SetIsInteractable(this, true);
		}
		
		LeverAtStartingPoint();
		
		if (!bIsBeingHeld && !CVarLeverTest.GetValueOnGameThread())
		{
			SetActorTickEnabled(false);
			GrabComponent->SetComponentTickEnabled(false);
		}
	}
	
	if (!bIsBeingHeld)
	{
		if (bShouldGoBackToInitialPositionWhenNotHeld)
		{
			const float CurrentRotationRoll = FMath::FInterpConstantTo(LeverHandleMeshComponent->GetRelativeTransform().Rotator().Pitch, InitialOffRotationPitch, DeltaTime, ProgressResetSpeed);
			LeverHandleMeshComponent->SetRelativeRotation(FRotator(CurrentRotationRoll, 0.0f, 0.0f), true);
		}
		return;
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistWheel.h"

#include "Components/AudioComponent.h"
#include "Core/DetachableGrabComponent.h"
#include "Core/HeistGrabComponent.h"
#include "Core/HeistTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"


static TAutoConsoleVariable<bool> CVarWheelTest(TEXT("game.lever.testWheel"), false, TEXT("Enable test wheel for turning manually in editor and testing interaction functionality."), ECVF_Cheat);

AHeistWheel::AHeistWheel()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bShouldGoBackToInitialPositionWhenNotHeld = true;
	
	bIsRemoteGrabbable = false;
	
	TargetFullRotationRoll = 500.0f;
	InitialOffRotationRoll = 0.0f;
	
	BaseMeshComponent->SetCollisionProfileName("VR_Grabbable");
	BaseMeshComponent->ComponentTags.Add("Detachable");
	BaseMeshComponent->SetSimulatePhysics(false);
	
	WheelMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelMeshComp"));
	WheelMeshComponent->SetupAttachment(BaseMeshComponent);
	WheelMeshComponent->SetCollisionProfileName("VR_Grabbable");
	
	HandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleMeshComp"));
	HandleMeshComponent->SetupAttachment(WheelMeshComponent);
	HandleMeshComponent->SetCollisionProfileName("VR_Grabbable");
	
	GrabComponent->DestroyComponent();
	GrabComponent = CreateDefaultSubobject<UDetachableGrabComponent>(TEXT("GrabComp"));
	GrabComponent->SetupAttachment(HandleMeshComponent);
	GrabComponent->InitializeGrabComponent(HandleMeshComponent, true);
	GrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	GrabComponent->SetSimulateOnDrop(false);
	
	ProgressResetSpeed = 60.0f;
	InterpToPlayerHandSpeed = 100.0f;
	HandRotationThresholdToMove = 0.6f;
	
	CheckForPlayerEverySecondsForStopTick = 2.0f;
	
	bIsHandRotationFixed = false;
	bIsSetAmount1or2 = false;
	
	ValveAudioComponent = CreateDefaultSubobject<UAudioComponent>("ValveAudioComp");
	ValveAudioComponent->SetupAttachment(BaseMeshComponent);
}

void AHeistWheel::ChangeLinkedActor(AActor* NewLinkedActor)
{
	LinkedActor = NewLinkedActor;
}

float AHeistWheel::GetProgressNormalized() const
{
	return (CurrentRotationRoll - InitialOffRotationRoll) / (TargetFullRotationRoll - InitialOffRotationRoll);
}

void AHeistWheel::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentRotationRoll = InitialOffRotationRoll;
	WheelMeshComponent->SetRelativeRotation(FRotator(CurrentRotationRoll, 0.0f, 0.0f));
	InitialQuat = WheelMeshComponent->GetComponentQuat();
	LastQuat = InitialQuat;
}

void AHeistWheel::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	
	UHeistMotionControllerComponent* MotionControllerRef = GrabComponent->GetCurrentMotionControllerHoldingThis();
	
	if (!MotionControllerRef) return;
	const FTransform ControllerTransform = MotionControllerRef->GetComponentTransform();
	const FTransform BaseTransform = BaseMeshComponent->GetComponentTransform();
	
	FTransform ControllerTransformRelativeToLeverBase = ControllerTransform.GetRelativeTransform(BaseTransform);
	
	FVector DirectionFromWheelToHand = ControllerTransformRelativeToLeverBase.GetTranslation();
	DirectionFromWheelToHand.X = 0.0f;
	DirectionFromWheelToHand.Normalize();
	// const FVector WheelDirectionTowardsHandle = UKismetMathLibrary::GetRightVector(WheelTransform.Rotator());
	
	FVector WheelDirectionTowardsHandle =
	BaseTransform.InverseTransformVector(
		WheelMeshComponent->GetRightVector()
	);

	WheelDirectionTowardsHandle.X = 0.0f;
	WheelDirectionTowardsHandle.Normalize();
	
	
	// const float DeltaRotation = (DirectionFromWheelToHand - WheelDirectionTowardsHandle).Rotation().Pitch;
	
	// DrawDebugLine(GetWorld(), WheelTransform.GetTranslation(), WheelTransform.GetTranslation() + (DirectionFromWheelToHand * 50.0f), FColor::Red, false, 0.1f, 0, 20.0f);
	// DrawDebugLine(GetWorld(), WheelTransform.GetTranslation(), WheelTransform.GetTranslation() + (WheelDirectionTowardsHandle * 50.0f), FColor::Blue, false, 0.1f, 0, 20.0f);
	
	// const float Dott = FVector::DotProduct(WheelDirectionTowardsHandle, DirectionFromWheelToHand);
	
	const float SignedAngleRad = FMath::Atan2(
	FVector::CrossProduct(WheelDirectionTowardsHandle, DirectionFromWheelToHand).X,
	FVector::DotProduct(WheelDirectionTowardsHandle, DirectionFromWheelToHand)
	);
	float SignedAngleDeg = FMath::RadiansToDegrees(SignedAngleRad) - 180.0f;
	
	if (!FMath::IsNearlyZero(SignedAngleRad, HandRotationThresholdToMove))
	{
		// const float RotationValueBeforeClamp = (-ControllerTransformRelativeToLeverBase.GetTranslation()).ToOrientationQuat().Rotator().Pitch;
		// CurrentRotationRoll = FMath::Clamp(RotationValueBeforeClamp, InitialOffRotationRoll, TargetFullRotationRoll);
	
		// CurrentRotationRoll = FMath::FInterpConstantTo(CurrentRotationRoll, CurrentRotationRoll + DeltaRotation, DeltaTime, 70.0f);
	
		CurrentRotationRoll = FMath::Clamp(FMath::FInterpConstantTo(CurrentRotationRoll, CurrentRotationRoll + (UKismetMathLibrary::NormalizeAxis(SignedAngleDeg)), DeltaTime, InterpToPlayerHandSpeed)
			, InitialOffRotationRoll, TargetFullRotationRoll);
		// UE_LOG(LogTemp, Log, TEXT("%f"),  CurrentRotationRoll);
		const FRotator FinalRotation = FRotator(0.0f, 0, CurrentRotationRoll);
	
		WheelMeshComponent->SetRelativeRotation(FinalRotation);
	}
	
	
	const FTransform HandleTransform = HandleMeshComponent->GetComponentTransform();
	
	ControllerTransformRelativeToLeverBase = ControllerTransform.GetRelativeTransform(HandleTransform);
	
	if (!bIsHandRotationFixed)
		ControllerTransformRelativeToLeverBase.SetRotation(HandRotationOffset.Quaternion());
	ControllerTransformRelativeToLeverBase.SetTranslation(HandLocationOffset);
	
	ControllerTransformRelativeToLeverBase = ControllerTransformRelativeToLeverBase * HandleTransform;
	
	MotionControllerRef->PhysicsHandRef->SetWorldLocationAndRotation(ControllerTransformRelativeToLeverBase.GetTranslation()
		 , bIsHandRotationFixed ? HandRotationOffset.Quaternion() : ControllerTransformRelativeToLeverBase.GetRotation(), false, nullptr, ETeleportType::ResetPhysics);
}

UPrimitiveComponent* AHeistWheel::GetMainPrimitiveComponent() const
{
	return BaseMeshComponent;
	// return Super::GetMainPrimitiveComponent();
}

void AHeistWheel::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	if (LinkedActor)
		bIsRemoteGrabbable = false;
	
	Super::OnPlayerChangeSize(NewPlayerSize);
	const bool bActive = CurrentSize == NewPlayerSize;
	BaseMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	BaseMeshComponent->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	if (!LinkedActor)
	{
		if (bActive)
		{
			BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			HandleMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			BaseMeshComponent->SetSimulatePhysics(true);
		}
		else
		{
			BaseMeshComponent->SetSimulatePhysics(false);
			BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			HandleMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	BaseMeshComponent->SetVisibility(bActive, true);
	
	if (LinkedActor)
	{
		HandleMeshComponent->SetSimulatePhysics(false);
		BaseMeshComponent->SetSimulatePhysics(false);
		WheelMeshComponent->SetSimulatePhysics(false);
	}
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

void AHeistWheel::CheckForPlayerStopTick()
{
	if (bShouldGoBackToInitialPositionWhenNotHeld)
	{
		if (FMath::IsNearlyEqual(CurrentRotationRoll, InitialOffRotationRoll))
		{
			GetWorldTimerManager().ClearTimer(CheckForPlayerStopTickTimerHandle);
			SetActorTickEnabled(false);
		}
	}
	else if (FMath::IsNearlyZero(WheelMeshComponent->GetPhysicsAngularVelocityInDegrees().Length(), 1.f))
	{
		WheelMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		GetWorldTimerManager().ClearTimer(CheckForPlayerStopTickTimerHandle);
		SetActorTickEnabled(false);
	}
}

void AHeistWheel::OnWheelTouchedOrHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor->IsA(APawn::StaticClass()))
	{
		// Player hit it.
		SetActorTickEnabled(true);
		
		GetWorldTimerManager().ClearTimer(CheckForPlayerStopTickTimerHandle);
		GetWorldTimerManager().SetTimer(CheckForPlayerStopTickTimerHandle, this, &AHeistWheel::CheckForPlayerStopTick, CheckForPlayerEverySecondsForStopTick, bShouldGoBackToInitialPositionWhenNotHeld);
	}
}

void AHeistWheel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const bool bIsInteractable = Execute_GetIsInteractable(this);
	const bool bIsBeingHeld = GrabComponent->IsBeingHeld();
	
	
	// WheelMeshComponent->SetRelativeRotation(FRotator(CurrentRotationRoll, 0.0f, 0.0f), true, nullptr, ETeleportType::ResetPhysics);
	
	const float ProgressNormalized = GetProgressNormalized();
	
	
	// 
	
	
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
		
		WheelAtStartingPoint();
		
		if (!bIsBeingHeld && !CVarWheelTest.GetValueOnGameThread())
		{
			SetActorTickEnabled(false);
			ValveAudioComponent->SetBoolParameter("OnTurnStop", true);
			GrabComponent->SetComponentTickEnabled(false);
		}
		
		return;
	}
	
	
	
	if (!bIsBeingHeld)
	{
		if (bShouldGoBackToInitialPositionWhenNotHeld)
		{
			CurrentRotationRoll = FMath::FInterpConstantTo(CurrentRotationRoll, InitialOffRotationRoll, DeltaTime, ProgressResetSpeed);
			WheelMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, CurrentRotationRoll), false);
		}
		return;
	}
	
}

void AHeistWheel::OnWheelGrabbed(UHeistGrabComponent* GrabbedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	if (BaseMeshComponent->IsSimulatingPhysics()) return;
	
	SetActorTickEnabled(true);
	GrabComponent->SetComponentTickEnabled(true);
	
	IHeistInteractionInterface::Execute_SetIsInFocus(LinkedActor, true);
	ValveAudioComponent->SetBoolParameter("OnTurnStart", true);
	
	
	if (GrabComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", false, false);
	else
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", false, false);
}

void AHeistWheel::OnWheelReleased(UHeistGrabComponent* ReleasedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	if (BaseMeshComponent->IsSimulatingPhysics()) return;
	
	IHeistInteractionInterface::Execute_SetIsInFocus(LinkedActor, false);
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

bool AHeistWheel::IsGrabbable_Implementation(const FName BoneHit) const
{
	return bIsGrabbableActive;
}

void AHeistWheel::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		ValveAudioComponent->Play();
		const bool bIsTiny = CurrentSize == EHeistSize::TINY;
		if (!LinkedActor)
		{
			GrabComponent->PrimitiveComponent = BaseMeshComponent;
			GrabComponent->AttachToComponent(BaseMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			
			if (!bIsTiny)
			{
				BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				BaseMeshComponent->SetSimulatePhysics(true);
			}
			
			GrabComponent->GrabTypeBase = EGrabTypeBase::FREE;
			GrabComponent->SetSimulateOnDrop(true);
		}
		
		if (bIsTiny)
		{
			BaseMeshComponent->SetVisibility(false, true);
			BaseMeshComponent->SetSimulatePhysics(false);
			BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		
		GrabComponent->OnGrabbed.AddDynamic(this, &AHeistWheel::OnWheelGrabbed);
		GrabComponent->OnReleased.AddDynamic(this, &AHeistWheel::OnWheelReleased);
		// WheelMeshComponent->OnComponentHit.AddDynamic(this, &AHeistWheel::OnWheelTouchedOrHit);
		
		StartingScale = GetActorScale3D();
	}
}

void AHeistWheel::AttachToNewAnchorPoint(USceneComponent* NewAnchorToAttachTo)
{
	bIsRemoteGrabbable = false;
	ForceRelease();
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&, NewAnchorToAttachTo]()
	{
		HandleMeshComponent->SetSimulatePhysics(false);
		BaseMeshComponent->SetSimulatePhysics(false);
		WheelMeshComponent->SetSimulatePhysics(false);
		bIsRemoteGrabbable = false;
		CastChecked<UDetachableGrabComponent>(GrabComponent)->AttachToAnchorPoint(this, NewAnchorToAttachTo, GrabComponent->CurrentMotionControllerHoldingThis);
		HandleMeshComponent->SetSimulatePhysics(false);
		BaseMeshComponent->SetSimulatePhysics(false);
		WheelMeshComponent->SetSimulatePhysics(false);
	});
	GetWorldTimerManager().SetTimerForNextTick(TimerDelegate);

}

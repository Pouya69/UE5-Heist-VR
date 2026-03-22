// Fill out your copyright notice in the Description page of Project Settings.


#include "SizeChangeMachine.h"

#include "Components/SplineComponent.h"
#include "Core/DetachableGrabComponent.h"
#include "Core/HeistFunctionLibrary.h"
#include "Core/HeistGameMode.h"
#include "Core/HeistTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"


static TAutoConsoleVariable<bool> CVarSizeMachineProgressTest(TEXT("game.sizeMachine.progressTest"), false, TEXT("If true, you can play with the spline value slider in the instance."), ECVF_Cheat);

ASizeChangeMachine::ASizeChangeMachine()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	bCanChangeSize = false;
	
	Machine_SK_Component = CreateDefaultSubobject<USkeletalMeshComponent>("Machine_SK_Comp");
	SetRootComponent(Machine_SK_Component);
	
	BaseMeshComponent->DestroyComponent();
	
	GrabComponent->SetupAttachment(Machine_SK_Component, "hand_l");
	GrabComponent->InitializeGrabComponent(Machine_SK_Component);
	GrabComponent->SetSimulateOnDrop(false);
	Machine_SK_Component->SetSimulatePhysics(false);
	GrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	
	SecondGrabComponent = CreateDefaultSubobject<UHeistGrabComponent>("SecondGrabComp");
	
	SecondGrabComponent->SetupAttachment(Machine_SK_Component, "hand_r");
	SecondGrabComponent->InitializeGrabComponent(Machine_SK_Component);
	SecondGrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	SecondGrabComponent->SetSimulateOnDrop(false);
	
	LeftHandMovementSplineComponent = CreateDefaultSubobject<USplineComponent>("LeftHandMovementSplineComp");
	LeftHandMovementSplineComponent->SetupAttachment(Machine_SK_Component);
	
	RightHandMovementSplineComponent = CreateDefaultSubobject<USplineComponent>("RightHandMovementSplineComp");
	RightHandMovementSplineComponent->SetupAttachment(Machine_SK_Component);
	
	TeleportDestinationSceneComponent = CreateDefaultSubobject<USceneComponent>("TeleportDestinationSceneComp");
	TeleportDestinationSceneComponent->SetupAttachment(Machine_SK_Component);
	TeleportDestinationSceneComponent->bAutoActivate = false;
	TeleportDestinationSceneComponent->SetComponentTickEnabled(false);
	
	Max_Y_Distance_FromGrabComps = 60.0f;
	
	SplineResetSpeed = 0.6f;
	
	bIsRemoteGrabbable = false;
	
	AcceptableBonesToGrab.Add(TEXT("hand_r"));
	AcceptableBonesToGrab.Add(TEXT("hand_l"));
	AcceptableBonesToGrab.Add(TEXT("joint4_l"));
	AcceptableBonesToGrab.Add(TEXT("joint4_r"));
	
	bIsGrabbableActive = true;
	
	TrueSizeForTiny = FVector(0.001f, 0.001f, 0.001f);
}

bool ASizeChangeMachine::IsGrabbableBasedOnBoneHit(const FName BoneHit) const
{
	if (AcceptableBonesToGrab.IsEmpty()) return true;
	
	return BoneHit != NAME_None && AcceptableBonesToGrab.Contains(BoneHit);
}

FVector ASizeChangeMachine::GetNewPlayerLocationAfterTeleport() const
{
	FTransform TeleportTransform = TeleportDestinationSceneComponent->GetComponentTransform();
	const float Multiplier = UHeistFunctionLibrary::GetSizeMultiplierBasedOnType(CurrentSize);
	TeleportTransform.SetTranslationAndScale3D(TeleportTransform.GetTranslation() * Multiplier, TeleportTransform.GetScale3D() * Multiplier);
	return TeleportTransform.GetTranslation();
}


bool ASizeChangeMachine::IsGrabbable_Implementation(const FName BoneHit) const
{
	return bIsGrabbableActive && IsGrabbableBasedOnBoneHit(BoneHit);
}

void ASizeChangeMachine::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	Super::OnPlayerChangeSize(NewPlayerSize);
	Machine_SK_Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CurrentSize == EHeistSize::TINY)
	{
		Machine_SK_Component->SetSkeletalMesh(MediumMeshRef, false);
		if (NewPlayerSize == EHeistSize::TINY)
			SetActorScale3D(FVector(1.16f, 1.16f, 1.16f));
		else
			SetActorScale3D(StartingScale);
		// Machine_SK_Component->SetCollisionEnabled(CurrentSize == NewPlayerSize ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
	else
	{
		// Machine_SK_Component->SetSkeletalMesh(StartingMesh, false);
		// SetActorScale3D(StartingScale);
		// SetActorScale3D(TrueSizeForTiny);
	}
	
	FTimerDelegate Delegate;
	Delegate.BindLambda([&, NewPlayerSize]()
	{
		if (CurrentSize == NewPlayerSize)
		{
			Machine_SK_Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		LeftHandFinalLocation = LeftHandMovementSplineComponent->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World, true);
		RightHandFinalLocation = RightHandMovementSplineComponent->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World, true);
	});
	
	GetWorldTimerManager().SetTimerForNextTick(Delegate);
	
	
}

void ASizeChangeMachine::OnPlayerChangeSize_02(EHeistSize NewPlayerSize)
{
		/*
	const bool bActive = NewPlayerSize == CurrentSize;
	const bool bIsTiny = CurrentSize == EHeistSize::TINY;
	if (bIsTiny)
	{
		if (NewPlayerSize == EHeistSize::TINY)
		{
			SetActorScale3D(StartingScale * 1000.0f);
			SetActorLocation(GetActorLocation() * MEDIUM_SIZE_MULT, false, nullptr, ETeleportType::TeleportPhysics);
		}
		else
		{
			SetActorScale3D(StartingScale);
			SetActorLocation(GetActorLocation() * 0.001f, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
*/
}

void ASizeChangeMachine::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		TeleportTransformDestination = TeleportDestinationSceneComponent->GetComponentTransform();
		
		GrabComponent->OnGrabbed.AddDynamic(this, &ASizeChangeMachine::OnHandleGrabbed);
		SecondGrabComponent->OnGrabbed.AddDynamic(this, &ASizeChangeMachine::OnHandleGrabbed);
	
		GrabComponent->OnReleased.AddDynamic(this, &ASizeChangeMachine::OnHandleReleased);
		SecondGrabComponent->OnReleased.AddDynamic(this, &ASizeChangeMachine::OnHandleReleased);
	
		if (!CVarSizeMachineProgressTest.GetValueOnGameThread())
		{
			LeftHandFinalLocation = LeftHandMovementSplineComponent->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World, true);
			RightHandFinalLocation = RightHandMovementSplineComponent->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World, true);
		}
		
		StartingMesh = Machine_SK_Component->GetSkeletalMeshAsset();
		
		// AHeistGameMode* GM = Cast<AHeistGameMode>(GetWorld()->GetAuthGameMode());
		// GM->OnPlayerChangeSize.AddDynamic(this, &ASizeChangeMachine::OnPlayerChangeSize_02);
		
		const bool bActive = CurrentSize != EHeistSize::TINY;
		// Machine_SK_Component->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		Machine_SK_Component->SetVisibility(bActive, true);
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
	UGameplayStatics::PlaySound2D(this, ChangeSizeSound);
	
	Machine_SK_Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	const bool bSuccess = IHeistPlayerInterface::Execute_ChangeSize(UGameplayStatics::GetPlayerPawn(this, 0), MachinePlayerSizeChange, GetNewPlayerLocationAfterTeleport());
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
	
	if (GrabComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
	{
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", false, false);
	}
	else
	{
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", false, false);
	}
	Machine_SK_Component->Activate();
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
	
	if (GrabComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
	{
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", true, false);
	}
	else
	{
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", true, false);
	}
}

void ASizeChangeMachine::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	if (IsReadyToUseSizeMachine())
	{
		Execute_Interact(this);
		return;
	}
	
	const FTransform BaseTransform = Machine_SK_Component->GetComponentTransform();
	
	if (GrabComponent->IsBeingHeld())
	{
		UHeistMotionControllerComponent* ControllerRef = GrabComponent->GetCurrentMotionControllerHoldingThis();
		const FTransform LeftHandStartTransformWorld = LeftHandStartTransform * BaseTransform;
		const FTransform ControllerTransform = ControllerRef->GetComponentTransform();
		
		FTransform ControllerTransformRelativeToGrabComp = ControllerTransform.GetRelativeTransform(LeftHandStartTransformWorld);

		FVector HandPosition = ControllerTransform.GetTranslation();
		const float SplineKey = LeftHandMovementSplineComponent->FindInputKeyClosestToWorldLocation(HandPosition);
		
		LeftSplineNormalizeProgress = LeftHandMovementSplineComponent->GetTimeAtDistanceAlongSpline(LeftHandMovementSplineComponent->GetDistanceAlongSplineAtSplineInputKey(SplineKey));
		
		LeftHandFinalLocation = LeftHandMovementSplineComponent->GetLocationAtTime(LeftSplineNormalizeProgress, ESplineCoordinateSpace::World, true);
		
		bIsLeftHandReady = FMath::IsNearlyEqual(LeftSplineNormalizeProgress, 1.0f);
		
		ControllerTransformRelativeToGrabComp.SetTranslation(LeftHandFinalLocation + GrabOffset);
		ControllerTransformRelativeToGrabComp.SetRotation(FQuat::MakeFromEuler(GrabOffsetRotation));
		
		ControllerRef->PhysicsHandRef->SetWorldTransform(ControllerTransformRelativeToGrabComp);
		
	}
	
	if (SecondGrabComponent->IsBeingHeld())
	{
		const FTransform RightHandStartTransformWorld = RightHandStartTransform * BaseTransform;
		UHeistMotionControllerComponent* ControllerRef = SecondGrabComponent->GetCurrentMotionControllerHoldingThis();
		const FTransform ControllerTransform = ControllerRef->GetComponentTransform();
		FTransform ControllerTransformRelativeToGrabComp = ControllerRef->GetComponentTransform().GetRelativeTransform(RightHandStartTransformWorld);
		
		FVector HandPosition = ControllerTransform.GetTranslation();
		const float SplineKey = RightHandMovementSplineComponent->FindInputKeyClosestToWorldLocation(HandPosition);
		
		RightSplineNormalizeProgress = RightHandMovementSplineComponent->GetTimeAtDistanceAlongSpline(RightHandMovementSplineComponent->GetDistanceAlongSplineAtSplineInputKey(SplineKey));
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

bool ASizeChangeMachine::GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents)
{
	if (GrabComponent && SecondGrabComponent && GrabComponent->IsGrabComponentReady() && SecondGrabComponent->IsGrabComponentReady())
	{
		OutGrabComponents.Add(GrabComponent);
		OutGrabComponents.Add(SecondGrabComponent);
		
		return true;
	}
	
	return false;
}

void ASizeChangeMachine::BeginPlay()
{
	Super::BeginPlay();
	
	if (CurrentSize == EHeistSize::TINY)
	{
		// StartingScale = TrueSizeForTiny;
		Machine_SK_Component->SetCollisionEnabled(CurrentSize == EHeistSize::MEDIUM ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
	
	// OnPlayerChangeSize(EHeistSize::MEDIUM);
	
	RightHandStartTransform = SecondGrabComponent->GetRelativeTransform();
	LeftHandStartTransform = GrabComponent->GetRelativeTransform();
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
		Machine_SK_Component->Deactivate();
		SetActorTickEnabled(false);
		return;
	}
}

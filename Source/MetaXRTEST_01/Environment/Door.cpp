// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"

#include "Core/HeistGrabComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"

static TAutoConsoleVariable<bool> CVarDoorOpen(TEXT("game.doors.TestDoorOpen"), false, TEXT("Toggles on Close and Open a door."), ECVF_Cheat);
static TAutoConsoleVariable<bool> CVarDebugDrawHandleAndHandRelation(TEXT("game.doors.DebugDrawHandleAndHandRelation"), false, TEXT("Toggles on the debug drawing of hands and handle properties."), ECVF_Cheat);


ADoor::ADoor()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// Base Mesh here is the anchor.
	BaseMeshComponent->SetCanEverAffectNavigation(false);
	
	// DO NOT FORGET TO LOCK POSITION ON ALL AXIS AND LOCK ROTATION ON X AND Y
	DoorMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMeshComp"));
	DoorMeshComponent->SetupAttachment(BaseMeshComponent);
	DoorMeshComponent->SetSimulatePhysics(false);  // Door should not be simulated while the handle hasn't been pushed down.
	DoorMeshComponent->SetCanEverAffectNavigation(false);
	
	HandleAnchorComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandleAnchorComp"));
	HandleAnchorComponent->SetupAttachment(DoorMeshComponent);
	HandleAnchorComponent->PrimaryComponentTick.bCanEverTick = false;
	HandleAnchorComponent->SetComponentTickEnabled(false);
	
	DoorHandleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorHandleComp"));
	DoorHandleComponent->SetupAttachment(HandleAnchorComponent);
	DoorHandleComponent->SetCanEverAffectNavigation(false);
	
	GrabComponent->SetupAttachment(DoorHandleComponent);
	GrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	GrabComponent->InitializeGrabComponent(DoorHandleComponent);
	GrabComponent->SetSimulateOnDrop(false);
	
	DoorHandleComponent->SetSimulatePhysics(false);
	
	// MAKE SURE TO ROTATE THE DOORCONSTRAINT CORRECTLY INITIALLY IN BP
	DoorConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("DoorConstraintComp"));
	DoorConstraint->SetupAttachment(BaseMeshComponent);
	DoorConstraint->ComponentName1 = FConstrainComponentPropName(TEXT("BaseMeshComp"));
	DoorConstraint->ComponentName2 = FConstrainComponentPropName(TEXT("DoorMeshComp"));
	DoorConstraint->SetAngularTwistLimit(ACM_Locked, 0.f);
	DoorConstraint->SetAngularSwing2Limit(ACM_Locked, 0.f);
	DoorConstraint->SetAngularSwing1Limit(ACM_Limited, 80.f);  // 80 Degrees on either side.
	DoorConstraint->SetLinearXLimit(LCM_Free, 0.f);
	DoorConstraint->SetLinearYLimit(LCM_Free, 0.f);
	DoorConstraint->SetLinearZLimit(LCM_Free, 0.f);
	DoorConstraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	DoorConstraint->SetAngularOrientationDrive(true, false);
	DoorConstraint->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	
	DoorClosedDifferenceYawThreshold = 1.f;
	
	bIsHandlePushedDown = false;
	DoorHandleMaxPitch = -70.0f;
	DoorHandleMinPitch = 0.0f;
	DoorOpenTriggerHandleThresholdPitch = -45.0f;
	
	MaxDistanceBetweenPhysicsHandAndMotionController = 30.f;
	
	bIsRemoteGrabbable = false;
}

void ADoor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	DoorClosedRotation = DoorMeshComponent->GetComponentQuat();  // Door has to be closed at the start.
	
	GrabComponent->OnGrabbed.AddDynamic(this, &ADoor::OnDoorHandleGrabbed);
	GrabComponent->OnReleased.AddDynamic(this, &ADoor::OnDoorHandleReleased);
	
}

void ADoor::OnDoorHandleGrabbed(UHeistGrabComponent* GrabbedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	// Not yet pushed down.
	
	// We start ticking to check for handle rotation.
	
	bIsHandOnHandle = true;
	
	SetActorTickEnabled(true);
	
	if (GrabbedComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", false, false);
	else
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", false, false);
}

void ADoor::OnDoorHandleReleased(UHeistGrabComponent* GrabbedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	bIsHandOnHandle = false;
	
	if (GrabbedComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", true, false);
	else
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", true, false);
}

void ADoor::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	UHeistMotionControllerComponent* MotionControllerHoldingHandle = GrabComponent->GetCurrentMotionControllerHoldingThis();
	/*
	if (!MotionControllerHoldingHandle)
	{
		// Door handle is not grabbed.
		
		return;
	}
	*/
	
	FTransform HandleAnchorTransform = HandleAnchorComponent->GetComponentTransform();
	
	const FTransform ControllerTransform = MotionControllerHoldingHandle->GetComponentTransform();
	
	FTransform ControllerTransformRelativeToAnchor = ControllerTransform.GetRelativeTransform(HandleAnchorTransform);
	
	const float FinalHandleAndHandPitchRotation = GrabComponent->GetHeldByHand() == EControllerHand::Left ? ControllerTransformRelativeToAnchor.GetRotation().Rotator().Pitch
	: -ControllerTransformRelativeToAnchor.GetRotation().Rotator().Pitch;
	
	bIsHandlePushedDown = FinalHandleAndHandPitchRotation <= DoorOpenTriggerHandleThresholdPitch;  // For being able to.
	
	if (bIsHandlePushedDown && !IsDoorOpen())
	{
		// We open the door once only. Not every frame simulating physics etc.
		OpenCloseDoor(true);
	}
	
	const FRotator FinalRotationHandle = FRotator(FinalHandleAndHandPitchRotation,0.0f,0.0f);
	
	HandleAnchorComponent->SetRelativeRotation(FinalRotationHandle, true, nullptr, ETeleportType::TeleportPhysics);
	
	ControllerTransformRelativeToAnchor.SetLocation(DoorHandleHandLocationOffset);
	ControllerTransformRelativeToAnchor.SetRotation(DoorHandleHandRotationOffset.Quaternion());

	
	ControllerTransformRelativeToAnchor = ControllerTransformRelativeToAnchor * HandleAnchorTransform;
	
	const FVector HandPosition = ControllerTransformRelativeToAnchor.GetTranslation();
	const FQuat HandRotation = ControllerTransformRelativeToAnchor.GetRotation();
	
	if (FVector::Dist(ControllerTransform.GetLocation(), HandPosition) > MaxDistanceBetweenPhysicsHandAndMotionController)
	{
		// Detach the hand due to the motion controller being too far from the supposed location.
		if (GrabComponent->IsBeingHeld())
			GrabComponent->TryRelease(MotionControllerHoldingHandle, UGameplayStatics::GetPlayerController(this, 0));
		return;
	}
	
	if (CVarDebugDrawHandleAndHandRelation.GetValueOnGameThread())
	{
		DrawDebugBox(GetWorld(), HandPosition, FVector(5,5,5), HandRotation, FColor::Green);
		DrawDebugDirectionalArrow(GetWorld(), HandPosition, ControllerTransform.GetLocation(), 15.f, FColor::Yellow);
	}
	
	MotionControllerHoldingHandle->PhysicsHandRef->SetWorldLocationAndRotation(HandPosition, HandRotation, true, nullptr, ETeleportType::ResetPhysics);
}

bool ADoor::IsDoorOpen() const
{
	return bIsDoorOpen;
}

void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// UE_LOG(LogTemp, Warning, TEXT("Door Tick"));
	
	if (!bIsHandOnHandle && !CVarDoorOpen.GetValueOnGameThread() && !CanDoorTick())
	{
		// HandleAnchorComponent->SetRelativeRotation(FMath::RInterpConstantTo(HandleAnchorComponent->GetRelativeTransform().GetRotation().Rotator(), FRotator::ZeroRotator, DeltaTime, 20.f));
		// Door should be closed. (When debugging the door, we disable this functionality at least)
		
		UE_LOG(LogTemp, Warning, TEXT("Door closing automatically."));
		OpenCloseDoor(false);
		return;
	}
	
	
	
	
	
}

bool ADoor::CanDoorRotate() const
{
	return bIsHandlePushedDown || CanDoorTick();
}

bool ADoor::CanDoorTick() const
{
	if (bIsHandOnHandle) return true;
	
	const float Difference = FMath::Abs(FMath::Abs(DoorClosedRotation.Rotator().Yaw) - FMath::Abs(DoorMeshComponent->GetComponentQuat().Rotator().Yaw));
	return !FMath::IsNearlyZero(Difference, DoorClosedDifferenceYawThreshold);
}

void ADoor::Interact_Implementation()
{
	// Door handle pushed down.
	
	
}

void ADoor::SetIsInteractable_Implementation(const bool bIsInteractable)
{
	
}

void ADoor::OpenCloseDoor(bool bOpen)
{
	bIsDoorOpen = bOpen;
	
	if (bOpen)
	{
		DoorMeshComponent->SetSimulatePhysics(true);
		DoorConstraint->InitComponentConstraint();
		SetActorTickEnabled(true);
		
		if (CVarDoorOpen.GetValueOnGameThread())
		{
			DoorMeshComponent->AddImpulse(-DoorMeshComponent->GetRightVector() * 20000.f);
		}
	}
	else
	{
		// Close
		DoorMeshComponent->SetSimulatePhysics(false);
		DoorMeshComponent->SetWorldRotation(DoorClosedRotation);
		
		SetActorTickEnabled(false);
		// DoorConstraint->InitializeComponent();
	}
	
	
	
	
		
}

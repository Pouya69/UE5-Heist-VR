// Fill out your copyright notice in the Description page of Project Settings.


#include "SlidingDoor.h"

#include "Core/HeistGrabComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"


// Sets default values
ASlidingDoor::ASlidingDoor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	DoorPhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("DoorPhysicsConstraintComp"));
	DoorPhysicsConstraint->SetupAttachment(GetRootComponent());
	DoorPhysicsConstraint->SetConstrainedComponents(BaseMeshComponent, NAME_None, nullptr, NAME_None);
	DoorPhysicsConstraint->SetLinearXLimit(LCM_Limited, 70.0f);
	DoorPhysicsConstraint->SetLinearYLimit(LCM_Locked, 0.0f);
	DoorPhysicsConstraint->SetLinearZLimit(LCM_Locked, 0.0f);
	DoorPhysicsConstraint->SetAngularSwing1Limit(ACM_Locked, 0.0f);
	DoorPhysicsConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
	DoorPhysicsConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);
	
	FirstHandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstHandleMeshComp"));
	FirstHandleMeshComponent->SetupAttachment(GetRootComponent());
	FirstHandleMeshComponent->SetCollisionProfileName("VR_Grabbable");
	FirstHandleMeshComponent->SetSimulatePhysics(false);
	
	SecondHandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondHandleMeshComp"));
	SecondHandleMeshComponent->SetupAttachment(GetRootComponent());
	SecondHandleMeshComponent->SetCollisionProfileName("VR_Grabbable");
	SecondHandleMeshComponent->SetSimulatePhysics(false);
	
	GrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	GrabComponent->SetupAttachment(FirstHandleMeshComponent);
	GrabComponent->SetSimulateOnDrop(true);
	GrabComponent->PrimitiveComponent = BaseMeshComponent;
	
	SecondHandleGrabComponent = CreateDefaultSubobject<UHeistGrabComponent>(TEXT("SecondGrabComp"));
	SecondHandleGrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	SecondHandleGrabComponent->SetupAttachment(SecondHandleMeshComponent);
	SecondHandleGrabComponent->SetSimulateOnDrop(true);
	SecondHandleGrabComponent->PrimitiveComponent = BaseMeshComponent;
	
	bIsRemoteGrabbable = false;
	
	DoorSlideInterpSpeedToController = 100.0f;
	
}

void ASlidingDoor::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	const FTransform ControllerTransform = WhichGrabComponent->CurrentMotionControllerHoldingThis->GetComponentTransform();
	UPrimitiveComponent* WhichHandle = WhichGrabComponent == GrabComponent ? FirstHandleMeshComponent : SecondHandleMeshComponent;
	const FTransform HandleTransform = WhichHandle->GetComponentTransform();
	
	FTransform ControllerTransformRelativeToHandle = ControllerTransform.GetRelativeTransform(HandleTransform);
	FVector ControllerLocation = ControllerTransformRelativeToHandle.GetTranslation();
	ControllerLocation.Y = 0.0f;
	ControllerLocation.Z = 0.0f;
	
	ControllerTransformRelativeToHandle.SetTranslation(ControllerLocation + HandLocationOffset);
	ControllerTransformRelativeToHandle.SetRotation((ControllerTransformRelativeToHandle.Rotator() + HandRotationOffset).Quaternion());
	
	FVector ControllerLocationRelative = ControllerTransformRelativeToHandle.GetTranslation();
	ControllerTransformRelativeToHandle = ControllerTransformRelativeToHandle * HandleTransform;
	
	BaseMeshComponent->AddForce(ControllerLocationRelative / 8.0f, NAME_None);
	
	WhichGrabComponent->CurrentMotionControllerHoldingThis->PhysicsHandRef->SetWorldLocationAndRotation(ControllerTransformRelativeToHandle.GetTranslation(), ControllerTransformRelativeToHandle.GetRotation(),
		false, nullptr, ETeleportType::TeleportPhysics);
	
	
}

void ASlidingDoor::ReleaseConstraintFromAnchor_Implementation(UPhysicsConstraintComponent* ReleasedConstraintComp)
{
	
}

void ASlidingDoor::OnHandleGrabbed(UHeistGrabComponent* GrabbedComponent,
                                   UHeistMotionControllerComponent* MotionControllerRef)
{
	GrabbedComponent->SetComponentTickEnabled(true);
	
	
	
	if (GrabbedComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
	{
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", false, false);
	}
	else
	{
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", false, false);
	}
	
	
}

void ASlidingDoor::OnHandleReleased(UHeistGrabComponent* ReleasedComponent,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	
	ReleasedComponent->SetComponentTickEnabled(false);
	
	
	if (GrabComponent->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", true, false);
	else
		IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", true, false);
	
}

bool ASlidingDoor::GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents)
{
	OutGrabComponents.Add(GrabComponent);
	OutGrabComponents.Add(SecondHandleGrabComponent);
	return true;
}

bool ASlidingDoor::IsGrabbable_Implementation(const FName BoneHit) const
{
	return true;
}

// Called when the game starts or when spawned
void ASlidingDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASlidingDoor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	GrabComponent->OnGrabbed.AddDynamic(this, &ASlidingDoor::OnHandleGrabbed);
	SecondHandleGrabComponent->OnGrabbed.AddDynamic(this, &ASlidingDoor::OnHandleGrabbed);
	GrabComponent->OnReleased.AddDynamic(this, &ASlidingDoor::OnHandleReleased);
	SecondHandleGrabComponent->OnReleased.AddDynamic(this, &ASlidingDoor::OnHandleReleased);
}

// Called every frame
void ASlidingDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


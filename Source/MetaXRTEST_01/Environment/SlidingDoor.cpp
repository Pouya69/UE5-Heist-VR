// Fill out your copyright notice in the Description page of Project Settings.


#include "SlidingDoor.h"

#include "Core/HeistGameMode.h"
#include "Core/HeistGrabComponent.h"
#include "Kismet/KismetMathLibrary.h"
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

bool ASlidingDoor::IsDoorJammed() const
{
	if (!LinkedConstraintComp) return false;
	
	return LinkedConstraintComp->ConstraintInstance.IsProjectionEnabled();
}

FVector ASlidingDoor::GetDoorOpenDirection() const
{
	return BaseMeshComponent->GetForwardVector();
}

void ASlidingDoor::SlideDoorOnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	DoorPhysicsConstraint->BreakConstraint();
	BaseMeshComponent->SetSimulatePhysics(false);
	
	if (CurrentSize == NewPlayerSize)
	{
		FTimerHandle Handle;
		FTimerDelegate Delegate;
		Delegate.BindLambda([&]()
		{
			BaseMeshComponent->SetSimulatePhysics(true);
			DoorPhysicsConstraint->SetConstrainedComponents(BaseMeshComponent, NAME_None, nullptr, NAME_None);
		});
	
		GetWorldTimerManager().SetTimer(Handle, Delegate, 0.2f, false);
	}
}

void ASlidingDoor::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	IHeistInteractionInterface::Execute_Custom_Tick(LinkedConstraintActor, DeltaTime, nullptr);
	
	const FTransform ControllerTransform = WhichGrabComponent->CurrentMotionControllerHoldingThis->GetComponentTransform();
	UPrimitiveComponent* WhichHandle = WhichGrabComponent == GrabComponent ? FirstHandleMeshComponent : SecondHandleMeshComponent;
	FTransform HandleTransform = WhichHandle->GetComponentTransform();
	
	FTransform ControllerTransformRelativeToHandle = ControllerTransform.GetRelativeTransform(HandleTransform);
	FVector ControllerLocation = ControllerTransformRelativeToHandle.GetTranslation();
	ControllerLocation.Y = 0.0f;
	ControllerLocation.X = 0.0f;
	if (IsDoorJammed())
	{
		ControllerLocation.Z = 0.0f;
	}
	
	ControllerTransformRelativeToHandle.SetTranslation(ControllerLocation);
	ControllerTransformRelativeToHandle.SetRotation(HandRotationOffset.Quaternion());
	
	FTransform NewControllerTransformRelativeToHandle = ControllerTransformRelativeToHandle * HandleTransform;
	
	const FVector Offset = WhichGrabComponent == SecondHandleGrabComponent ? BaseLocationOffsetFromHandle2 : BaseLocationOffsetFromHandle;
	FVector FinalDoorLocation = NewControllerTransformRelativeToHandle.GetTranslation() - Offset;
	BaseMeshComponent->SetWorldLocation(FinalDoorLocation, true, nullptr, ETeleportType::TeleportPhysics);
	
	HandleTransform = WhichHandle->GetComponentTransform();
	ControllerTransformRelativeToHandle = ControllerTransform.GetRelativeTransform(HandleTransform);
	ControllerLocation = ControllerTransformRelativeToHandle.GetTranslation();
	ControllerLocation.Y = 0.0f;
	ControllerLocation.X = 0.0f;
	ControllerLocation.Z = 0.0f;
	
	ControllerTransformRelativeToHandle.SetTranslation(ControllerLocation + HandLocationOffset);
	ControllerTransformRelativeToHandle.SetRotation(HandRotationOffset.Quaternion());
	ControllerTransformRelativeToHandle = ControllerTransformRelativeToHandle * HandleTransform;
	
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
	
	BaseMeshComponent->SetSimulatePhysics(false);
	DoorPhysicsConstraint->BreakConstraint();
	
	
	
	MotionControllerRef->PhysicsHandRef->IgnoreActorWhenMoving(this, true);
	BaseMeshComponent->IgnoreComponentWhenMoving(MotionControllerRef->PhysicsHandRef, true);
	FirstHandleMeshComponent->IgnoreComponentWhenMoving(MotionControllerRef->PhysicsHandRef, true);
	SecondHandleMeshComponent->IgnoreComponentWhenMoving(MotionControllerRef->PhysicsHandRef, true);
	
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
	
	
	FTimerDelegate Delegate;
	Delegate.BindLambda([&, MotionControllerRef]()
	{
		if (!IsDoorJammed())
		{
			BaseMeshComponent->SetSimulatePhysics(true);
			DoorPhysicsConstraint->SetConstrainedComponents(BaseMeshComponent, NAME_None, nullptr, NAME_None);
		}
		
		MotionControllerRef->PhysicsHandRef->IgnoreActorWhenMoving(this, false);
		BaseMeshComponent->IgnoreComponentWhenMoving(MotionControllerRef->PhysicsHandRef, false);
		FirstHandleMeshComponent->IgnoreComponentWhenMoving(MotionControllerRef->PhysicsHandRef, false);
		SecondHandleMeshComponent->IgnoreComponentWhenMoving(MotionControllerRef->PhysicsHandRef, false);
	});
	GetWorldTimerManager().SetTimerForNextTick(Delegate);
	
	
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
	
	if (LinkedConstraintActor && LinkedConstraintActor->Implements<UHeistInteractionInterface>())
		LinkedConstraintComp = IHeistInteractionInterface::Execute_GetPhysicsConstraintComp(LinkedConstraintActor);
	
	InitialDoorLocation = BaseMeshComponent->GetComponentLocation();
	
	BaseMeshComponent->SetSimulatePhysics(false);
	DoorPhysicsConstraint->BreakConstraint();
}

void ASlidingDoor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		GrabComponent->OnGrabbed.AddDynamic(this, &ASlidingDoor::OnHandleGrabbed);
		SecondHandleGrabComponent->OnGrabbed.AddDynamic(this, &ASlidingDoor::OnHandleGrabbed);
		GrabComponent->OnReleased.AddDynamic(this, &ASlidingDoor::OnHandleReleased);
		SecondHandleGrabComponent->OnReleased.AddDynamic(this, &ASlidingDoor::OnHandleReleased);
		
		BaseLocationOffsetFromHandle = FirstHandleMeshComponent->GetRelativeLocation();
		BaseLocationOffsetFromHandle2 = SecondHandleMeshComponent->GetRelativeLocation();
		
		AHeistGameMode* GM = Cast<AHeistGameMode>(GetWorld()->GetAuthGameMode());
		GM->OnPlayerChangeSize.AddDynamic(this, &ASlidingDoor::SlideDoorOnPlayerChangeSize);
	}
}

// Called every frame
void ASlidingDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


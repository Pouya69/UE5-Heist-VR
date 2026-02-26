// Fill out your copyright notice in the Description page of Project Settings.


#include "DetachableGrabComponent.h"

#include "Environment/HeistWheel.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"


// Sets default values for this component's properties
UDetachableGrabComponent::UDetachableGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	// PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDetachableGrabComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UDetachableGrabComponent::DetachWhenTooFarFromGrabbable()
{
	if (CurrentMotionControllerHoldingThis == nullptr || 
		FVector::Dist(CurrentMotionControllerHoldingThis->GetComponentLocation(), CurrentMotionControllerHoldingThis->PhysicsHandRef->GetComponentLocation()) <= HandDetachmentDistanceThreshold) return false;
	
	if (!bIsDetachable)
		return Super::DetachWhenTooFarFromGrabbable();
	
	UHeistMotionControllerComponent* ControllerRef = CurrentMotionControllerHoldingThis;
	
	if (AHeistWheel* WheelOwner = Cast<AHeistWheel>(GetOwner()))
	{
		WheelOwner->ChangeLinkedActor(nullptr);
		WheelOwner->bIsRemoteGrabbable = true;
		WheelOwner->BaseMeshComponent->SetSimulatePhysics(false);
		
		
		TryRelease(nullptr, UGameplayStatics::GetPlayerController(GetWorld(), 0));
		WheelOwner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GrabTypeBase = EGrabTypeBase::FREE;
		SetSimulateOnDrop(true);
		SetPrimitiveComponentPhysicsEnabled(true);
		SetComponentTickEnabled(false);
	}
	// We detach the object from Custom to FREE Grab Type so player can grab it.
	
	
	if (GetHeldByHand(ControllerRef) == EControllerHand::Left)
	{
		IHeistPlayerInterface::Execute_LeftForceGrab(ControllerRef->GetOwner());
	}
	else
	{
		IHeistPlayerInterface::Execute_RightForceGrab(ControllerRef->GetOwner());
	}
	
	return true;
	
}

void UDetachableGrabComponent::AttachToAnchorPoint(AGrabbable* OwnerGrabbable, USceneComponent* NewAnchorToAttachTo, UHeistMotionControllerComponent* ControllerRef)
{
	if (AHeistWheel* WheelOwner = Cast<AHeistWheel>(GetOwner()))
	{
		SetPrimitiveComponentPhysicsEnabled(false);
		PrimitiveComponent = WheelOwner->HandleMeshComponent;
		AttachToComponent(WheelOwner->HandleMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		
		WheelOwner->BaseMeshComponent->SetSimulatePhysics(false);
		WheelOwner->WheelMeshComponent->SetSimulatePhysics(false);
		WheelOwner->HandleMeshComponent->SetSimulatePhysics(false);
		// WheelOwner->BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetSimulateOnDrop(false);
		GrabTypeBase = EGrabTypeBase::CUSTOM;
		WheelOwner->BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		WheelOwner->WheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		WheelOwner->HandleMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);

	}
	
	
	
	OwnerGrabbable->AttachToComponent(NewAnchorToAttachTo, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false));
	
	if (ControllerRef == nullptr) return;
	// OwnerGrabbable->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	/*
	if (GetHeldByHand(ControllerRef) == EControllerHand::Left)
	{
		IHeistPlayerInterface::Execute_LeftForceGrab(ControllerRef->GetOwner());
	}
	else
	{
		IHeistPlayerInterface::Execute_RightForceGrab(ControllerRef->GetOwner());
	}
	*/
}


// Called every frame
void UDetachableGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

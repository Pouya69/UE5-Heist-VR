// Fill out your copyright notice in the Description page of Project Settings.


#include "SizeChangeMachine.h"

#include "Core/HeistGrabComponent.h"
#include "Core/HeistTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Player/HeistPlayerInterface.h"


ASizeChangeMachine::ASizeChangeMachine()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	FirstHandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("FirstHandleMeshComp");
	FirstHandleMeshComponent->SetupAttachment(BaseMeshComponent);
	
	GrabComponent->SetupAttachment(FirstHandleMeshComponent);
	GrabComponent->InitializeGrabComponent(FirstHandleMeshComponent);
	GrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	
	SecondHandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("SecondHandleMeshComp");
	SecondHandleMeshComponent->SetupAttachment(BaseMeshComponent);
	
	SecondGrabComponent = CreateDefaultSubobject<UHeistGrabComponent>("SecondGrabComp");
	SecondGrabComponent->SetupAttachment(SecondHandleMeshComponent);
	SecondGrabComponent->InitializeGrabComponent(SecondHandleMeshComponent);
	SecondGrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	
	IsRemoteGrabbable = false;
	
}

void ASizeChangeMachine::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	GrabComponent->OnGrabbed.AddDynamic(this, &ASizeChangeMachine::OnHandleGrabbed);
	SecondGrabComponent->OnGrabbed.AddDynamic(this, &ASizeChangeMachine::OnHandleGrabbed);
	
	GrabComponent->OnReleased.AddDynamic(this, &ASizeChangeMachine::OnHandleReleased);
	SecondGrabComponent->OnReleased.AddDynamic(this, &ASizeChangeMachine::OnHandleReleased);
}

bool ASizeChangeMachine::IsReadyToPressButtons() const
{
	return GrabComponent->IsBeingHeld() && SecondGrabComponent->IsBeingHeld();
}

bool ASizeChangeMachine::IsReadyToUseSizeMachine() const
{
	return IsReadyToPressButtons() && bIsLeftOnePressed && bIsRightOnePressed;
}

void ASizeChangeMachine::PressedButtonForSizeChange(EControllerHand WhichHand, const bool bIsPressed)
{
	if (WhichHand == EControllerHand::Left)
	{
		bIsLeftOnePressed = bIsPressed;
	}
	else
	{
		bIsRightOnePressed = bIsPressed;
	}
	
	if (IsReadyToUseSizeMachine())
	{
		TimeGrabbedAndPressedBoth = UGameplayStatics::GetTimeSeconds(this);
		
		// @TODO: Play FX, sounds etc.
	}
}

bool ASizeChangeMachine::ChangePlayerSize(const EHeistSize NewPlayerSize)
{
	const bool bSuccess = IHeistPlayerInterface::Execute_ChangeSize(UGameplayStatics::GetPlayerPawn(this, 0), NewPlayerSize);
	
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
		SetActorTickEnabled(false);
	}
}

void ASizeChangeMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GrabComponent->IsBeingHeld())
	{
		UHeistMotionControllerComponent* ControllerRef = GrabComponent->GetCurrentMotionControllerHoldingThis();
		const FTransform ControllerTransform = ControllerRef->GhostHandRef->GetComponentTransform();
		FTransform ControllerTransformRelativeToGrabComp = ControllerRef->GhostHandRef->GetComponentTransform().GetRelativeTransform(GrabComponent->GetComponentTransform());
		
		ControllerTransformRelativeToGrabComp.SetTranslation(GrabOffset);
		ControllerTransformRelativeToGrabComp.SetRotation(FQuat::MakeFromEuler(GrabOffsetRotation));
		
		ControllerRef->PhysicsHandRef->SetWorldTransform(ControllerTransformRelativeToGrabComp);
	}
	
	if (SecondGrabComponent->IsBeingHeld())
	{
		UHeistMotionControllerComponent* ControllerRef = SecondGrabComponent->GetCurrentMotionControllerHoldingThis();
		const FTransform ControllerTransform = ControllerRef->GhostHandRef->GetComponentTransform();
		FTransform ControllerTransformRelativeToGrabComp = ControllerRef->GhostHandRef->GetComponentTransform().GetRelativeTransform(GrabComponent->GetComponentTransform());
		
		ControllerTransformRelativeToGrabComp.SetTranslation(GrabOffset);
		ControllerTransformRelativeToGrabComp.SetRotation(FQuat::MakeFromEuler(GrabOffsetRotation));
		
		ControllerRef->PhysicsHandRef->SetWorldTransform(ControllerTransformRelativeToGrabComp);
	}
}


#include "HeistRemoteDetonator.h"

#include "HeistDynamite.h"
#include "MotionControllerComponent.h"
#include "Core/HeistGrabComponent.h"
#include "Kismet/GameplayStatics.h"


AHeistRemoteDetonator::AHeistRemoteDetonator()
{
	PrimaryActorTick.bCanEverTick = true;
	
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	GrabComponent->GrabTypeBase = EGrabTypeBase::FREE;
	
	HandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleMeshComp"));
	HandleMeshComponent->SetupAttachment(BaseMeshComponent);
	
	HandleGrabComponent = CreateDefaultSubobject<UHeistGrabComponent>(TEXT("HandleGrabComp"));
	HandleGrabComponent->SetupAttachment(HandleMeshComponent);
	HandleGrabComponent->GrabTypeBase = EGrabTypeBase::FREE;
	HandleGrabComponent->InitializeGrabComponent(HandleMeshComponent);
	HandleGrabComponent->SetSimulateOnDrop(false);
	HandleGrabComponent->SetPrimitiveComponentPhysicsEnabled(false);
	
	HandleMinimumHeight = -30.0f;
	HandleMaximumHeight = 40.0f;
	HandleIdleHeight = 40.0f;
	HandleIdleHeightTransitionSpeed = 40.0f;
	
	DetonateAfterPushInSeconds = 0.8f;
	
	bCanDetonate = true;
}

void AHeistRemoteDetonator::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (!ensure(DynamiteLinked))
	{
		UE_LOG(LogTemp, Error, TEXT("Detonator %s does not have any Dynamite linked to it! FIX: Set Dynamite Linked in the properties."), *GetName());
		return;
	}
		
	MatchingRotationOffset = HandleMeshComponent->GetRelativeRotation();
	
	FVector HandleLocationRelative = HandleMeshComponent->GetRelativeLocation();
	HandleLocationRelative.Z = HandleIdleHeight;
	HandleMeshComponent->SetRelativeLocation(HandleLocationRelative);
	
	HandleGrabComponent->OnGrabbed.AddDynamic(this, &AHeistRemoteDetonator::OnHandleGrabbed);
	HandleGrabComponent->OnReleased.AddDynamic(this, &AHeistRemoteDetonator::OnHandleReleased);
}

void AHeistRemoteDetonator::Interact_Implementation()
{
	SetIsInteractable(false);
	UGameplayStatics::PlaySoundAtLocation(this, InteractSound, GetActorLocation());
	
	if (GetWorldTimerManager().IsTimerActive(DetonateTimerHandle)) return;
	
	if (DetonateAfterPushInSeconds <= 0.000f)
	{
		Detonate();
		return;
	}
	
	GetWorldTimerManager().SetTimer(DetonateTimerHandle, this, &AHeistRemoteDetonator::Detonate, DetonateAfterPushInSeconds, false);
	
}

bool AHeistRemoteDetonator::GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents)
{
	if (!GrabComponent || !GrabComponent->IsGrabComponentReady() || !HandleGrabComponent || !HandleGrabComponent->IsGrabComponentReady()) return false;
	
	OutGrabComponents.Add(GrabComponent);
	OutGrabComponents.Add(HandleGrabComponent);
	return true;
}


void AHeistRemoteDetonator::SetIsInteractable_Implementation(const bool bIsInteractable)
{
	bCanDetonate = bIsInteractable;
}

void AHeistRemoteDetonator::OnHandleGrabbed(UHeistGrabComponent* GrabComponentRef,
                                            UMotionControllerComponent* MotionControllerRef)
{
	SetActorTickEnabled(true);
}

void AHeistRemoteDetonator::OnHandleReleased(UHeistGrabComponent* GrabComponentRef,
	UMotionControllerComponent* MotionControllerRef)
{
	// We disable the ComponentTick after the idle height has been reached.
}

void AHeistRemoteDetonator::Detonate()
{
	DynamiteLinked->StartExplosion();
}

void AHeistRemoteDetonator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UMotionControllerComponent* MotionControllerHoldingHandle = HandleGrabComponent->GetCurrentMotionControllerHoldingThis();
	if (!MotionControllerHoldingHandle)
	{
		// Handle is not grabbed
		FVector HandleLocation = HandleMeshComponent->GetRelativeLocation();
		HandleLocation.Z = FMath::FInterpConstantTo(HandleLocation.Z, HandleIdleHeight, DeltaTime, HandleIdleHeightTransitionSpeed);
		HandleMeshComponent->SetRelativeLocation(HandleLocation);
		
		if (FMath::IsNearlyEqual(HandleLocation.Z, HandleIdleHeight))
		{
			// Reached the idle height.
			SetActorTickEnabled(false);
		}
		
		return;
	}
	// Handle is grabbed
	
	const FTransform HandleTransform = HandleMeshComponent->GetComponentTransform();
	
	const FTransform ControllerTransform = MotionControllerHoldingHandle->GetComponentTransform();
	
	FTransform ControllerTransformRelativeToHandle = ControllerTransform.GetRelativeTransform(
		FTransform(HandleTransform.Rotator() + MatchingRotationOffset, HandleTransform.GetTranslation(), HandleTransform.GetScale3D()));
	
	const FVector ControllerLocationRelative = ControllerTransformRelativeToHandle.GetTranslation();
	
	const float FinalZ = FMath::Clamp(ControllerLocationRelative.Z, HandleMinimumHeight, HandleMaximumHeight);
	const FVector FinalHeightVector = HandleTransform.GetTranslation() + 
		FVector(0.0f, 0.0f, FinalZ);
	HandleMeshComponent->SetRelativeLocation(FinalHeightVector);
	
	ControllerTransformRelativeToHandle.SetTranslation(ControllerLocationRelative + FinalHeightVector);
	// We set the rotation after so we cannot rotate the hand when it is locked on handle.
	ControllerTransformRelativeToHandle.SetRotation(HandleHandGrabbingRotationOffset.Quaternion());
	
	MotionControllerHoldingHandle->SetWorldTransform(ControllerTransformRelativeToHandle * HandleTransform);
	
	if (bCanDetonate && FMath::IsNearlyEqual(FinalZ, HandleMinimumHeight))
	{
		// Handle is at the bottom. Interact.
		Interact();
	}
	else if (FMath::IsNearlyEqual(FinalZ, HandleMaximumHeight))
	{
		// Handle is at the top. We recharge so we can interact now.
		SetIsInteractable(true);
	}
}


#include "HeistRemoteDetonator.h"

#include "HeistDynamite.h"
#include "Player/HeistMotionControllerComponent.h"
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
	HandleGrabComponent->GrabTypeBase = EGrabTypeBase::CUSTOM;
	HandleGrabComponent->InitializeGrabComponent(HandleMeshComponent);
	HandleGrabComponent->SetSimulateOnDrop(false);
	HandleGrabComponent->SetPrimitiveComponentPhysicsEnabled(false);
	
	HandleMinimumHeight = -30.0f;
	HandleMaximumHeight = 40.0f;
	HandleIdleHeight = 40.0f;
	HandleIdleHeightTransitionSpeed = 40.0f;
	
	BasePivotOffsetZ = -30.0f;
	
	DetonateAfterPushInSeconds = 0.8f;
	
	bCanDetonate = true;
}

void AHeistRemoteDetonator::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld() && !ensure(DynamiteLinked))
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
	Execute_SetIsInteractable(this, false);
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
                                            UHeistMotionControllerComponent* MotionControllerRef)
{
	SetActorTickEnabled(true);
	MotionControllerRef->SetComponentTickEnabled(false);
}

void AHeistRemoteDetonator::OnHandleReleased(UHeistGrabComponent* GrabComponentRef,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	MotionControllerRef->SetComponentTickEnabled(true);
	// We disable the ComponentTick after the idle height has been reached.
}

void AHeistRemoteDetonator::Detonate()
{
	DynamiteLinked->StartExplosion();
}

void AHeistRemoteDetonator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UHeistMotionControllerComponent* MotionControllerHoldingHandle = HandleGrabComponent->GetCurrentMotionControllerHoldingThis();
	if (!MotionControllerHoldingHandle)
	{
		// Handle is not grabbed
		FVector HandleLocation = HandleMeshComponent->GetRelativeLocation();
		HandleLocation.Z = FMath::FInterpConstantTo(HandleLocation.Z, HandleIdleHeight, DeltaTime, HandleIdleHeightTransitionSpeed);
		HandleMeshComponent->SetRelativeLocation(HandleLocation);
		
		if (FMath::IsNearlyEqual(HandleLocation.Z, HandleIdleHeight))
		{
			// Reached the idle height.
			// SetActorTickEnabled(false);
		}
		
		return;
	}
	// Handle is grabbed
	
	FVector HandPosition = FVector::ZeroVector;
	FQuat HandRotation = FQuat::Identity;
	
	FTransform BaseTransform = BaseMeshComponent->GetComponentTransform();
	
	const FTransform ControllerTransform = MotionControllerHoldingHandle->GetComponentTransform();
	
	FTransform ControllerTransformRelativeToBase = ControllerTransform.GetRelativeTransform(
		FTransform(BaseTransform.Rotator() + MatchingRotationOffset, BaseTransform.GetTranslation(), BaseTransform.GetScale3D()));
	
	const FVector BasePivotOffsetVector = FVector(0,0,BasePivotOffsetZ);
	FVector ControllerLocationRelative = ControllerTransformRelativeToBase.GetTranslation() + BasePivotOffsetVector;
	
	
	const float FinalZ = FMath::Clamp(ControllerLocationRelative.Z, HandleMinimumHeight, HandleMaximumHeight);
	ControllerLocationRelative.Z = FinalZ;
	UE_LOG(LogTemp, Warning, TEXT("Final Z: %f"), FinalZ);
	const FVector FinalHeightVector = FVector(0.0f, 0.0f, FinalZ);
	
	// MotionControllerHoldingHandle->SetWorldTransform(ControllerTransformRelativeToBase * BaseTransform);
	HandleMeshComponent->SetRelativeLocation(FinalHeightVector);
	
	ControllerTransformRelativeToBase.SetTranslation(FinalHeightVector - BasePivotOffsetVector);
	// We set the rotation after so we cannot rotate the hand when it is locked on handle.
	ControllerTransformRelativeToBase.SetRotation(HandleHandGrabbingRotationOffset.Quaternion());
	
	ControllerTransformRelativeToBase = ControllerTransformRelativeToBase * BaseTransform;
	
	HandPosition = ControllerTransformRelativeToBase.GetTranslation();
	HandRotation = ControllerTransformRelativeToBase.GetRotation();
	
	DrawDebugSphere(GetWorld(), HandPosition, 20.f, 10, FColor::Red);
	
	MotionControllerHoldingHandle->ConstrainTickGrab(DeltaTime, HandRotation, HandPosition, true);
	
	
	if (bCanDetonate && FMath::IsNearlyEqual(FinalZ, HandleMinimumHeight))
	{
		// Handle is at the bottom. Interact.
		// Execute_Interact(this);
	}
	else if (!bCanDetonate && FMath::IsNearlyEqual(FinalZ, HandleMaximumHeight))
	{
		// Handle is at the top. We recharge so we can interact now.
		Execute_SetIsInteractable(this, true);
	}
}


#include "HeistRemoteDetonator.h"

#include "HeistDynamite.h"
#include "Player/HeistMotionControllerComponent.h"
#include "Core/HeistGrabComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeistPlayerInterface.h"


static TAutoConsoleVariable<bool> CVar_DrawGrabDebug(TEXT("game.drawGrabDebug"), false, TEXT("Helps visualize the grabbing object's properties."), ECVF_Cheat);


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
	MaxDistanceBetweenControllerAndPhysicsHand = 50.f;
	
	BasePivotOffsetZ = -30.0f;
	
	DetonateAfterPushInSeconds = 0.6f;
	
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
	
	if (GrabComponentRef == HandleGrabComponent)
	{
		SetActorTickEnabled(true);
		if (GrabComponentRef->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
		{
			IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", false);
		}
		else
		{
			IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", false);
		}
		
		const float DotForwardHand = MotionControllerRef->GhostHandRef->GetRightVector().Dot(HandleGrabComponent->GetForwardVector());
		const float DotRightHand = MotionControllerRef->GhostHandRef->GetForwardVector().Dot(-HandleGrabComponent->GetRightVector());
		
		bIsInvertedForwardAndRight = DotForwardHand < 0 && DotRightHand < 0;
		
		UE_LOG(LogTemp, Warning, TEXT("Fwd: %f, Right: %f,s Inverted: %d"), DotForwardHand, DotRightHand, bIsInvertedForwardAndRight ? 1 : 0);
	}
	// MotionControllerRef->SetComponentTickEnabled(false);
}

void AHeistRemoteDetonator::OnHandleReleased(UHeistGrabComponent* GrabComponentRef,
	UHeistMotionControllerComponent* MotionControllerRef)
{
	if (GrabComponentRef == HandleGrabComponent)
	{
		HandleGrabComponent->CurrentMotionControllerHoldingThis = nullptr;
		
		if (GrabComponentRef->GetHeldByHand(MotionControllerRef) == EControllerHand::Left)
			IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightLeftHand_CPP(MotionControllerRef->GetOwner(), "hand_l", true);
		else
			IHeistPlayerInterface::Execute_SetupBonePhysicsAndWeightRightHand_CPP(MotionControllerRef->GetOwner(), "hand_r", true);
		
	}

	
	// MotionControllerRef->SetComponentTickEnabled(true);
	// We disable the ComponentTick after the idle height has been reached.
}

void AHeistRemoteDetonator::Detonate()
{
	if (DynamiteLinked)
		DynamiteLinked->StartExplosion();
}

void AHeistRemoteDetonator::Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent)
{
	UHeistMotionControllerComponent* MotionControllerHoldingHandle = HandleGrabComponent->GetCurrentMotionControllerHoldingThis();
	
	
	if (!GrabComponent->IsBeingHeld())
	{
		// Handle is being held but not the base.
		HandleMeshComponent->SetRelativeLocation(FVector(0, 0, HandleIdleHeight), true, nullptr, ETeleportType::ResetPhysics);
		SetActorTickEnabled(false);
		HandleGrabComponent->TryRelease(MotionControllerHoldingHandle, UGameplayStatics::GetPlayerController(this, 0));
		GrabComponent->TryRelease(nullptr, UGameplayStatics::GetPlayerController(this, 0));
		return;
	}
	
	
	
	const float DotForwardHand = MotionControllerHoldingHandle->GhostHandRef->GetRightVector().Dot(HandleGrabComponent->GetForwardVector());
	const float DotRightHand = MotionControllerHoldingHandle->GhostHandRef->GetForwardVector().Dot(-HandleGrabComponent->GetRightVector());
	
	if (bIsInvertedForwardAndRight != (DotForwardHand < 0 && DotRightHand < 0))
	{
		// Inverted we drop.
		HandleGrabComponent->TryRelease(MotionControllerHoldingHandle, UGameplayStatics::GetPlayerController(this, 0));
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
	const FVector FinalHeightVector = FVector(0.0f, 0.0f, FinalZ);
	
	if (bIsInvertedForwardAndRight)
	{
		ControllerTransformRelativeToBase.SetTranslation(FinalHeightVector - BasePivotOffsetVector + FVector(-HandleHandGrabbingLocationOffset.X, -HandleHandGrabbingLocationOffset.Y, HandleHandGrabbingLocationOffset.Z));
		ControllerTransformRelativeToBase.SetRotation((FRotator(0.f,180.f,0.f) + HandleHandGrabbingRotationOffset).Quaternion());
	}
	else
	{
		ControllerTransformRelativeToBase.SetTranslation(FinalHeightVector - BasePivotOffsetVector + HandleHandGrabbingLocationOffset);
		ControllerTransformRelativeToBase.SetRotation(HandleHandGrabbingRotationOffset.Quaternion());
	}
	
	ControllerTransformRelativeToBase = ControllerTransformRelativeToBase * BaseTransform;  // Convert it to world.
	
	HandPosition = ControllerTransformRelativeToBase.GetTranslation();
	
	
	
	// const float RotationDifference = HandRotationRelevant.AngularDistance(MotionControllerHoldingHandle->GhostHandRef->GetComponentQuat());
	// UE_LOG(LogTemp, Warning, TEXT("Rotation Difference. %f"), RotationDifference);
	
	
	if (FVector::Dist(HandPosition, ControllerTransform.GetTranslation()) > MaxDistanceBetweenControllerAndPhysicsHand)
	{
		// Physics hand location is too far from max allowed distance. We stop holding.
		
		HandleGrabComponent->TryRelease(MotionControllerHoldingHandle, UGameplayStatics::GetPlayerController(this, 0));
		return;
	}
	
	HandleMeshComponent->SetRelativeLocation(FinalHeightVector);
	
	
	HandRotation = ControllerTransformRelativeToBase.GetRotation();
	
	if (CVar_DrawGrabDebug.GetValueOnGameThread())
	{
		DrawDebugSphere(GetWorld(), HandPosition, 5.f, 10, FColor::Red);
	}
	
	
	MotionControllerHoldingHandle->PhysicsHandRef->SetWorldLocationAndRotation(HandPosition, HandRotation, false, nullptr, ETeleportType::ResetPhysics);
	
	// MotionControllerHoldingHandle->ConstrainTickGrab(DeltaTime, HandRotation, HandPosition, true);
	
	
	if (bCanDetonate && FMath::IsNearlyEqual(FinalZ, HandleMinimumHeight))
	{
		// Handle is at the bottom. Interact.
		Execute_Interact(this);
	}
	else if (!bCanDetonate && FMath::IsNearlyEqual(FinalZ, HandleMaximumHeight))
	{
		// Handle is at the top. We recharge so we can interact now.
		Execute_SetIsInteractable(this, true);
	}
}

void AHeistRemoteDetonator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!HandleGrabComponent->GetCurrentMotionControllerHoldingThis())
	{
		// Handle is not grabbed or the base is not grabbed.
		FVector HandleLocation = HandleMeshComponent->GetRelativeLocation();
		HandleLocation.Z = FMath::FInterpConstantTo(HandleLocation.Z, HandleIdleHeight, DeltaTime, HandleIdleHeightTransitionSpeed);
		HandleMeshComponent->SetRelativeLocation(HandleLocation, true, nullptr, ETeleportType::ResetPhysics);
		
		if (FMath::IsNearlyEqual(HandleLocation.Z, HandleIdleHeight, 0.01f))
		{
			// Reached the idle height.
			SetActorTickEnabled(false);
		}
	}
}

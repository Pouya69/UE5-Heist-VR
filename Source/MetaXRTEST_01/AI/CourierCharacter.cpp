

#include "CourierCharacter.h"

#include "CourierController.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Core/HeistTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeistPlayerInterface.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ACourierCharacter::ACourierCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ObstacleAttackForceBone = "RightHand";
	ObstacleCheckRadius = 20.0f;
	
	ObstaclePushAwayForce = 50000.0f;
	
	bIsInDefaultCutsceneActions = true;
	
	CurrentSize = EHeistSize::MEDIUM;
}

void ACourierCharacter::SetCutsceneAlpha(float NewAlpha)
{
	CutsceneAlpha = NewAlpha;
}

void ACourierCharacter::SetLookAtPlayerAlpha(float NewAlpha)
{
	LookAtPlayerAlpha = NewAlpha;
}

void ACourierCharacter::InterruptedCutscene_Implementation(const bool bEndCurrentCutscene)
{
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	
	CourierControllerRef->GetBlackboardComponent()->SetValueAsBool("Is In Cutscene", true);
	CourierControllerRef->GetBlackboardComponent()->ClearValue("Is Moving To Destination");
	CourierControllerRef->GetBlackboardComponent()->ClearValue("Is Blocked By Obstacle");
	CourierControllerRef->GetBlackboardComponent()->ClearValue("Target Destination Actor");

	CourierControllerRef->GetBlackboardComponent()->ClearValue("Current Tracking Object");
	
	Sprint(false);
	
	CutsceneAlpha = 0.0f;
	// LookAtPlayerAlpha = 1.0f;
	
	
	if (bEndCurrentCutscene)
	{
		CurrentCutscenePlaying = nullptr;
		ULevelSequencePlayer* CurrentCutsceneRef = Cast<ULevelSequencePlayer>(CourierControllerRef->GetBlackboardComponent()->GetValueAsObject("Current Cutscene Player Ref"));
		if (CurrentCutsceneRef)
			CurrentCutsceneRef->Stop();
	}
	
	bIsInDefaultCutsceneActions = false;
	
	CourierControllerRef->GetBlackboardComponent()->ClearValue("Current Cutscene Player Ref");
}

void ACourierCharacter::Sprint(const bool bIsSprinting)
{
	BaseMovementSpeed = (bIsSprinting ? 150.0f : 80.0f);
	
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed * (CurrentSize == EHeistSize::TINY ? MEDIUM_SIZE_MULT : 1.0f);
}

void ACourierCharacter::FocusOnPlayer(const float NewAlpha)
{
	CurrentLookingAtTarget = PlayerCameraRefSceneComponent;
	
	LookAtPlayerAlpha = NewAlpha;
}

void ACourierCharacter::ClearFocus()
{
	CurrentLookingAtTarget = nullptr;
	LookAtPlayerAlpha = 0.0f;
}

void ACourierCharacter::StartAttackObstacle()
{
	PlayAnimMontage(AttackObstacleMontage);
}

void ACourierCharacter::AttackObstacle()
{
	const FVector Start = GetMesh()->GetSocketLocation("ObstacleAttackForceBone");
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	
	const bool bDidHitObstacle = GetWorld()->SweepSingleByObjectType(HitResult, Start, Start, FQuat::Identity, ALWAYS_COLLIDING_PHYSICS_ACTOR_CHANNEL,
		FCollisionShape::MakeSphere(ObstacleCheckRadius), CollisionParams);
	
	ensure(bDidHitObstacle);
	
	HitResult.GetComponent()->AddImpulseAtLocation(-HitResult.ImpactNormal * ObstaclePushAwayForce, HitResult.ImpactPoint);
}

void ACourierCharacter::ObjectSlowedDown_Implementation()
{
	if (CurrentCutscenePlaying && CurrentCutscenePlaying->GetSequencePlayer()->IsPlaying())
	{
		// Slow down the cutscene only.
		CurrentCutscenePlaying->GetSequencePlayer()->SetPlayRate(0.1f);
	}
	else
	{
		// Not in cutscene.
	}
}

void ACourierCharacter::ObjectOutOfSlowMotion_Implementation()
{
	if (CurrentCutscenePlaying && CurrentCutscenePlaying->GetSequencePlayer()->IsPlaying())
	{
		// Slow down the cutscene only.
		CurrentCutscenePlaying->GetSequencePlayer()->SetPlayRate(1.0f);
	}
	else
	{
		// Not in cutscene.
	}
}

bool ACourierCharacter::IsGrabbable_Implementation(const FName BoneHit) const
{
	return false;
}

bool ACourierCharacter::IsRemoteGrabbable_Implementation() const
{
	return false;
}

EHeistSize ACourierCharacter::GetCurrentSizeOfGameObject_Implementation()
{
	return CurrentSize;
}

void ACourierCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		CourierControllerRef = Cast<ACourierController>(GetController());
		BaseMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}
}

void ACourierCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerPawnRef = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawnRef && PlayerPawnRef->Implements<UHeistPlayerInterface>())
		PlayerCameraRefSceneComponent = IHeistPlayerInterface::Execute_GetPlayerCameraReferenceSoft(PlayerPawnRef);
}

void ACourierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentSize != EHeistSize::TINY || !GetCharacterMovement()->IsFalling()) return;
	
	FVector MyVelocity = GetCharacterMovement()->Velocity;
	MyVelocity.Z -= 980000.0f;
	// UE_LOG(LogTemp, Warning, TEXT("Move Velocity: %f"), MyVelocity.Z);
	
	GetCharacterMovement()->Velocity = MyVelocity;
	
}

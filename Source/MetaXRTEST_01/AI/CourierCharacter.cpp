

#include "CourierCharacter.h"

#include "CourierController.h"
#include "Core/HeistTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeistPlayerInterface.h"

#include "MotionWarpingComponent.h"


ACourierCharacter::ACourierCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ObstacleAttackForceBone = "RightHand";
	ObstacleCheckRadius = 20.0f;
	
	ObstaclePushAwayForce = 50000.0f;
	
	MotionWarpComponent = CreateDefaultSubobject<UMotionWarpingComponent>("MotionWarpComp");
}

void ACourierCharacter::SetCutsceneAlpha(float NewAlpha)
{
	CutsceneAlpha = NewAlpha;
}

void ACourierCharacter::SetLookAtPlayerAlpha(float NewAlpha)
{
	LookAtPlayerAlpha = NewAlpha;
}

void ACourierCharacter::FocusOnPlayer()
{
	CurrentLookingAtTarget = PlayerCameraRefSceneComponent;
	LookAtPlayerAlpha = 1.0f;
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

void ACourierCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		CourierControllerRef = Cast<ACourierController>(GetController());
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
}

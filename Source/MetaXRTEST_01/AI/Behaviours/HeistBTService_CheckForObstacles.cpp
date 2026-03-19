// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistBTService_CheckForObstacles.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/CourierCharacter.h"
#include "Core/HeistTypes.h"

UHeistBTService_CheckForObstacles::UHeistBTService_CheckForObstacles()
{
	NodeName = TEXT("Check For Obstacles");
	
	ObstacleCheckDistance = 10.0f;
	ObstacleCheckRadius = 15.0f;
}

void UHeistBTService_CheckForObstacles::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                 float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	ACourierCharacter* CourierCharacterRef = Cast<ACourierCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	
	const FVector Start = CourierCharacterRef->GetActorLocation();
	const FVector End = Start + CourierCharacterRef->GetActorForwardVector() * ObstacleCheckDistance;
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CourierCharacterRef);
	
	const bool bIsBlocked = GetWorld()->SweepSingleByObjectType(HitResult, Start, End, FQuat::Identity,
	ALWAYS_COLLIDING_PHYSICS_ACTOR_CHANNEL, FCollisionShape::MakeSphere(ObstacleCheckRadius), CollisionParams) || (GetWorld()->SweepSingleByObjectType(HitResult, Start, End, FQuat::Identity,
	ECC_Vehicle, FCollisionShape::MakeSphere(ObstacleCheckRadius), CollisionParams) && HitResult.GetActor()->ActorHasTag("Jammed Door"));
	
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(IsBlockedByObstacleKey.SelectedKeyName, bIsBlocked);
	
}

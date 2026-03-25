// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistBTTask_MoveDirectlyToward.h"

#include "AI/CourierController.h"
#include "AI/CourierCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UHeistBTTask_MoveDirectlyToward::UHeistBTTask_MoveDirectlyToward()
{
	NodeName = TEXT("Custom Move Directly Toward");
	
	bIsDestinationAnObject = true;
	MovementSpeed = -1.0f;
	AcceptableRadius = 35.0f;
}

EBTNodeResult::Type UHeistBTTask_MoveDirectlyToward::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACourierCharacter* CourierRef = Cast<ACourierCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	
	const AActor* ObjectRef = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetDestination.SelectedKeyName));
	
	if (bIsDestinationAnObject && ObjectRef == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	FVector DestinationLocation = bIsDestinationAnObject ? ObjectRef->GetActorLocation()
	: BlackboardComp->GetValueAsVector(TargetDestination.SelectedKeyName);
	
	const FVector CourierLocation = CourierRef->GetActorLocation();
	
	if (bDistance2DOnly)
	{
		if (FVector::DistSquared2D(DestinationLocation, CourierLocation) <= FMath::Square(AcceptableRadius))
		{
			return EBTNodeResult::Succeeded;
		}
		
		DestinationLocation.Z = CourierLocation.Z;
	}
	else
	{
		if (FVector::DistSquared(DestinationLocation, CourierLocation) <= FMath::Square(AcceptableRadius))
		{
			return EBTNodeResult::Succeeded;
		}
	}
	
	// Movement Towards Destination.
	
	CourierRef->GetCharacterMovement()->Velocity = (DestinationLocation - CourierRef->GetActorLocation()).GetUnsafeNormal() * CourierRef->GetCharacterMovement()->MaxWalkSpeed;
	
	return EBTNodeResult::InProgress;
}

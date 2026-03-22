// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistBTService_CheckIfMoving.h"

#include "AIController.h"
#include "AI/CourierCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UHeistBTService_CheckIfMoving::UHeistBTService_CheckIfMoving()
{
	NodeName = "Check If Moving";
	
	VelocityThresholdCheck = 0.5f;
}

void UHeistBTService_CheckIfMoving::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	ACourierCharacter* CourierRef = CastChecked<ACourierCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	const bool bIsMoving = CourierRef->GetCharacterMovement()->Velocity.Length() >= VelocityThresholdCheck;
	if (bIsMoving) return;
	
	// FVector DestinationLocation = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Target Destination Actor"))->GetActorLocation();
	// const FVector CourierLocation = CourierRef->GetActorLocation();
	// DestinationLocation.Z = CourierLocation.Z;
	
	OwnerComp.GetBlackboardComponent()->SetValueAsObject("Target Destination Actor", CourierRef);
	
}

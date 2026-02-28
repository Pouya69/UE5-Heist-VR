// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistBTTask_AttackObstacle.h"

#include "AIController.h"
#include "AI/CourierCharacter.h"

EBTNodeResult::Type UHeistBTTask_AttackObstacle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Cast<ACourierCharacter>(OwnerComp.GetAIOwner()->GetPawn())->StartAttackObstacle();
	
	return EBTNodeResult::Succeeded;
}

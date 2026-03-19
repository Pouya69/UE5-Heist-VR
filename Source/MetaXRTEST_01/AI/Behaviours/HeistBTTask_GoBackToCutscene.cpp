// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistBTTask_GoBackToCutscene.h"

#include "AIController.h"
#include "LevelSequencePlayer.h"
#include "AI/CourierCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UHeistBTTask_GoBackToCutscene::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	OwnerComp.GetBlackboardComponent()->SetValueAsBool("Is In Cutscene", true);
	OwnerComp.GetBlackboardComponent()->ClearValue("Is Moving To Destination");
	OwnerComp.GetBlackboardComponent()->ClearValue("Is Blocked By Obstacle");

	OwnerComp.GetBlackboardComponent()->ClearValue("Current Tracking Object");
	
	ULevelSequencePlayer* CurrentCutsceneRef = Cast<ULevelSequencePlayer>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Current Cutscene Player Ref"));
	OwnerComp.GetBlackboardComponent()->ClearValue("Current Cutscene Player Ref");
	
	ACourierCharacter* Courier = CastChecked<ACourierCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	Courier->Sprint(false);
	
	CurrentCutsceneRef->Play();
	
	return EBTNodeResult::Succeeded;
}

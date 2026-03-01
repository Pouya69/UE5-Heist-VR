// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistBTTask_GoBackToCutscene.h"

#include "LevelSequencePlayer.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UHeistBTTask_GoBackToCutscene::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	OwnerComp.GetBlackboardComponent()->SetValueAsBool("Is Moving To Destination", false);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool("Is In Cutscene", true);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool("Is Blocked By Obstacle", false);
	
	ULevelSequencePlayer* CurrentCutsceneRef = Cast<ULevelSequencePlayer>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Current Cutscene Player Ref"));
	OwnerComp.GetBlackboardComponent()->SetValueAsObject("Current Cutscene Player Ref", nullptr);
	CurrentCutsceneRef->Play();
	
	return EBTNodeResult::Succeeded;
}

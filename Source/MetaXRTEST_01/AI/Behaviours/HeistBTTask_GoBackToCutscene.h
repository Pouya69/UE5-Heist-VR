// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "HeistBTTask_GoBackToCutscene.generated.h"

/**
 * 
 */
UCLASS()
class METAXRTEST_01_API UHeistBTTask_GoBackToCutscene : public UBTTaskNode
{
	GENERATED_BODY()
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

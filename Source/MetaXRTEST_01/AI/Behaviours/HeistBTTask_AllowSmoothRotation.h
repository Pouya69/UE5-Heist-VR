// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "HeistBTTask_AllowSmoothRotation.generated.h"

/**
 * 
 */
UCLASS()
class METAXRTEST_01_API UHeistBTTask_AllowSmoothRotation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UHeistBTTask_AllowSmoothRotation();
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Rotation")
		bool bUseDesiredRotation;
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

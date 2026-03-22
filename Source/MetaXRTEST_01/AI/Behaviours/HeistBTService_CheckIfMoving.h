// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "HeistBTService_CheckIfMoving.generated.h"

/**
 * 
 */
UCLASS()
class METAXRTEST_01_API UHeistBTService_CheckIfMoving : public UBTService
{
	GENERATED_BODY()
	
public:
	UHeistBTService_CheckIfMoving();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
		float VelocityThresholdCheck;
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};

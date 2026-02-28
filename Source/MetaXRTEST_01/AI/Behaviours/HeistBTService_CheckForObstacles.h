// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "HeistBTService_CheckForObstacles.generated.h"

/**
 * 
 */
UCLASS()
class METAXRTEST_01_API UHeistBTService_CheckForObstacles : public UBTService
{
	GENERATED_BODY()
	
public:
	UHeistBTService_CheckForObstacles();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Obstacles")
		FBlackboardKeySelector IsBlockedByObstacleKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Obstacles")
		float ObstacleCheckDistance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Obstacles")
		float ObstacleCheckRadius;
	
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};

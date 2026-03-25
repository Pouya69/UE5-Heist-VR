// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "HeistBTTask_MoveDirectlyToward.generated.h"

/**
 * 
 */
UCLASS()
class METAXRTEST_01_API UHeistBTTask_MoveDirectlyToward : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UHeistBTTask_MoveDirectlyToward();
	
protected:
	// If true, it will get the value from blackboard as Object instead of Vector
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		bool bIsDestinationAnObject;
	
	// If true, only 2D distance will be counted.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		bool bDistance2DOnly;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		FBlackboardKeySelector TargetDestination;
	
	// If <= 0, it will use MaxWalkSpeed in CharacterMovementComponent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		float MovementSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		float AcceptableRadius;
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};


#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "CourierController.generated.h"

class UBehaviorTree;

UCLASS(Abstract)
class METAXRTEST_01_API ACourierController : public AAIController
{
	GENERATED_BODY()

public:
	ACourierController();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Behaviours")
		TObjectPtr<UBehaviorTree> CourierBehaviourDefault;
	
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;

public:
	virtual void Tick(float DeltaTime) override;
};

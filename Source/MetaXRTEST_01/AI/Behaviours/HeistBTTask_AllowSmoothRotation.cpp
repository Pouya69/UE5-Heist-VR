

#include "HeistBTTask_AllowSmoothRotation.h"

#include "AIController.h"
#include "AI/CourierCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UHeistBTTask_AllowSmoothRotation::UHeistBTTask_AllowSmoothRotation()
{
	NodeName = "Allow Smooth Rotation";
}

EBTNodeResult::Type UHeistBTTask_AllowSmoothRotation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACourierCharacter* CourierCharacter = Cast<ACourierCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	CourierCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = bUseDesiredRotation;
	
	return EBTNodeResult::Succeeded;
}

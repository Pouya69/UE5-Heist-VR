
#pragma once

#include "CoreMinimal.h"
#include "Core/Grabbable.h"
#include "HeistLever.generated.h"

enum class EHeistObjectInteractionType : uint8;

UCLASS(Abstract)
class METAXRTEST_01_API AHeistLever : public AGrabbable
{
	GENERATED_BODY()

public:
	AHeistLever();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Linked Object")
		TObjectPtr<AActor> LinkedActor;
	
	// 0 is not turned at all, 1 is full turned. (InitialOffRotationPitch -> TargetFullRotationPitch)
	UFUNCTION(BlueprintCallable, Category="Progress")
		float GetProgressNormalized() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		float InitialOffRotationPitch;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		float TargetFullRotationPitch;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		bool bShouldGoBackToInitialPositionWhenNotHeld;
	
	// If true, SetAmount for conitnous interaction and movement of objects is done
	// If not, only Interact()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		EHeistObjectInteractionType LeverInteractionType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		float ProgressResetSpeed;
	
	bool bIsLeverInteractable;
	
	UFUNCTION(BlueprintCallable, Category="Linked Object")
		void ToggleLeverEnabled(const bool bEnabled);
	
	virtual void OnPlayerChangeSize(EHeistSize NewPlayerSize) override;
	
protected:
	virtual bool GetIsInteractable_Implementation() const override;
	virtual void SetIsInteractable_Implementation(const bool bIsInteractable) override;
	
	virtual void Interact_Implementation() override;
	
	virtual void PostInitializeComponents() override;
	
	UFUNCTION()
		void OnLeverGrabbed(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION()
		void OnLeverReleased(UHeistGrabComponent* ReleasedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	// For TargetPoint, we have Interact() as it is in the interface.
	// For back to start, if we want any extra stuff.
	UFUNCTION(BlueprintImplementableEvent, Category="Progress")
		void LeverAtStartingPoint();
	
	virtual void Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> LeverHandleMeshComponent;
	
	UPROPERTY(EditAnywhere, Category="Hand Config")
		FVector HandLocationOffset;
	
	UPROPERTY(EditAnywhere, Category="Hand Config")
		float HandLocationOffset_FromLeverForward;
	
	UPROPERTY(EditAnywhere, Category="Hand Config")
		FRotator HandRotationOffset;
	
public:
	virtual void Tick(float DeltaTime) override;
};

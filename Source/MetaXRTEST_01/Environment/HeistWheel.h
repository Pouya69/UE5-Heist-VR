// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Grabbable.h"
#include "HeistWheel.generated.h"

enum class EHeistObjectInteractionType : uint8;

UCLASS(Abstract)
class METAXRTEST_01_API AHeistWheel : public AGrabbable
{
	GENERATED_BODY()

public:
	AHeistWheel();
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Linked Object")
		TObjectPtr<AActor> LinkedActor;
	
	// 0 is not turned at all, 1 is full turned. (InitialOffRotationRoll -> TargetFullRotationRoll)
	UFUNCTION(BlueprintCallable, Category="Progress")
		float GetProgressNormalized() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		float InitialOffRotationRoll;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		float TargetFullRotationRoll;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Progress")
		float CurrentRotationRoll;
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		bool bShouldGoBackToInitialPositionWhenNotHeld;
	
	virtual void Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent) override;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Progress")
		void WheelAtStartingPoint();
	
	virtual void Interact_Implementation() override;
	
	virtual bool GetIsInteractable_Implementation() const override;
	virtual void SetIsInteractable_Implementation(const bool bIsInteractable) override;
	
	UPROPERTY(BlueprintReadOnly, Category="Progress")
		bool bIsWheelInteractable;
	
	// If true, SetAmount for conitnous interaction and movement of objects is done
	// If not, only Interact()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		EHeistObjectInteractionType LeverInteractionType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		float ProgressResetSpeed;

protected:
	UPROPERTY(EditAnywhere, Category="Hand Config")
		FVector HandLocationOffset;
	
	UPROPERTY(EditAnywhere, Category="Hand Config")
		FRotator HandRotationOffset;
public:
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
		void OnWheelGrabbed(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION()
		void OnWheelReleased(UHeistGrabComponent* ReleasedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	
};

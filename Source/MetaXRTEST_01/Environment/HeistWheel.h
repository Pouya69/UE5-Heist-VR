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
	
	UFUNCTION(BlueprintCallable, Category="Linked Object")
		void AttachToNewAnchorPoint(USceneComponent* NewAnchorToAttachTo);
	
	// For when it is attached to a new point etc.
	UFUNCTION(BlueprintCallable, Category="Linked Object")
		void ChangeLinkedActor(AActor* NewLinkedActor);
	
	// False -> 1, true -> 2
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
		bool bIsSetAmount1or2;
	
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
	
	virtual UPrimitiveComponent* GetMainPrimitiveComponent() const override;
	
	virtual void OnPlayerChangeSize(EHeistSize NewPlayerSize) override;
	
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> HandleMeshComponent;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> WheelMeshComponent;
	
protected:
	// Will trigger a timer based on this time. For stopping ticking.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress | Tick")
		float CheckForPlayerEverySecondsForStopTick;
	
	UPROPERTY(EditAnywhere, Category="Hand Config")
		float InterpToPlayerHandSpeed;
	
	UPROPERTY(EditAnywhere, Category="Hand Config")
		float HandRotationThresholdToMove;
	
	FTimerHandle CheckForPlayerStopTickTimerHandle;
	
	FQuat InitialQuat;
	FQuat LastQuat;
	
	UFUNCTION()
		void CheckForPlayerStopTick();
	
	UFUNCTION()
		void OnWheelTouchedOrHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	
	UPROPERTY(EditAnywhere, Category="Hand Config")
		FVector HandLocationOffset;
	
	UPROPERTY(EditAnywhere, Category="Hand Config")
		FRotator HandRotationOffset;
				
	// If true, hand will be always at the rotation offset and NOT relative to wheel and rotate with the wheel.
	UPROPERTY(EditAnywhere, Category="Hand Config")
        bool bIsHandRotationFixed;
	
	UFUNCTION()
		void OnWheelGrabbed(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION()
		void OnWheelReleased(UHeistGrabComponent* ReleasedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	virtual bool IsGrabbable_Implementation(const FName BoneHit) const override;
	
	virtual void PostInitializeComponents() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Grabbable.h"
#include "SizeChangeMachine.generated.h"

class USplineComponent;
enum class EHeistSize : uint8;
class UHeistMotionControllerComponent;

UCLASS()
class METAXRTEST_01_API ASizeChangeMachine : public AGrabbable
{
	GENERATED_BODY()

public:
	ASizeChangeMachine();
	
	UFUNCTION(BlueprintCallable, Category = "Size Change Machine")
		bool IsReadyToUseSizeMachine() const;
	
	UFUNCTION(BlueprintCallable, Category = "Size Change Machine")
		bool IsReadyToPressButtons() const;
	
	UFUNCTION(BlueprintCallable, Category = "Size Change Machine")
		bool ChangePlayerSize();
	
	UPROPERTY(EditAnywhere, Category = "Size Change Machine")
		EHeistSize MachinePlayerSizeChange;
	
	UPROPERTY(BlueprintReadOnly, Category = "Size Change Machine")
		FVector LeftHandFinalLocation;
	
	UPROPERTY(BlueprintReadOnly, Category = "Size Change Machine")
		FVector RightHandFinalLocation;
	
	virtual bool IsGrabbable_Implementation(const FName BoneHit) const override;
	
	bool IsGrabbableBasedOnBoneHit(const FName BoneHit) const;

protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UHeistGrabComponent> SecondGrabComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<USkeletalMeshComponent> Machine_SK_Component;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<USplineComponent> RightHandMovementSplineComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<USplineComponent> LeftHandMovementSplineComponent;
	
	UPROPERTY(EditAnywhere, Category = "Size Change Machine | Grab Config")
		FVector GrabOffset;
	
	UPROPERTY(EditAnywhere, Category = "Size Change Machine | Grab Config")
		FVector GrabOffsetRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size Change Machine | Grab Config")
		float Max_Y_Distance_FromGrabComps;
	
	UPROPERTY(BlueprintReadOnly, Category = "Size Change Machine | Grab Config")
		bool bIsRightHandReady;
	
	UPROPERTY(BlueprintReadOnly, Category = "Size Change Machine | Grab Config")
		bool bIsLeftHandReady;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Size Change Machine", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float LeftSplineNormalizeProgress;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Size Change Machine", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float RightSplineNormalizeProgress;
	
	// How fast it should go back to 0 if not grabbed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size Change Machine")
		float SplineResetSpeed;
	
	UFUNCTION()
		void OnHandleGrabbed(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION()
		void OnHandleReleased(UHeistGrabComponent* ReleasedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	virtual void Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent) override;
	
	virtual void Interact_Implementation() override;
	
	// Only if we hit these bones we grab. If empty, it will be grabbable everywhere.
	UPROPERTY(EditAnywhere, Category="Skeletons")
		TArray<FName> AcceptableBonesToGrab;

public:
	virtual void Tick(float DeltaTime) override;
};

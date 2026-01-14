// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HeistGrabComponent.generated.h"

// For custom, we can define our own Enums to be used.
UENUM(BlueprintType)
enum class EGrabTypeBase : uint8
{
	NONE,
	FREE,
	SNAP,
	CUSTOM
};


class UHeistMotionControllerComponent;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrabbed, UHeistGrabComponent*, GrabComponent, UHeistMotionControllerComponent*, MotionControllerRef);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReleased, UHeistGrabComponent*, GrabComponent, UHeistMotionControllerComponent*, MotionControllerRef);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class METAXRTEST_01_API UHeistGrabComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHeistGrabComponent();

protected:
	
	// HAS TO BE SET USING: InitializeGrabComponent()
	UPROPERTY(BlueprintReadOnly, Category="Grab Component | OtherComponents")
		TObjectPtr<UPrimitiveComponent> PrimitiveComponent;
	
	UPROPERTY(BlueprintReadOnly, Category="Grab Component | OtherComponents")
		TObjectPtr<UHeistMotionControllerComponent> CurrentMotionControllerHoldingThis;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		bool bSimulateOnDrop;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool AttachPrimitiveCompToMotionController(UHeistMotionControllerComponent* MotionController);
	
	UPROPERTY(BlueprintReadOnly, Category="Grab Component")
		bool bIsHeld;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		TObjectPtr<UHapticFeedbackEffect_Base> OnGrabHapticEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		TObjectPtr<USoundBase> OnGrabSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		TObjectPtr<UHapticFeedbackEffect_Base> OnReleaseHapticEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		TObjectPtr<USoundBase> OnReleaseSound;
	
public:
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void TraceFingersProcedural();
	
	// Call this in PostInitializeComponents() of the actor
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void InitializeGrabComponent(UPrimitiveComponent* InPrimitiveComp, const bool bWasInitializedFromActor = false);
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		UHeistMotionControllerComponent* GetCurrentMotionControllerHoldingThis();
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool IsGrabComponentReady() const;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool TryGrab(UHeistMotionControllerComponent* MotionController, APlayerController* PlayerController);
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool TryRelease(UHeistMotionControllerComponent* MotionController, APlayerController* PlayerController);
	
	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Grab Component")
		FOnGrabbed OnGrabbed;
	
	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Grab Component")
		FOnReleased OnReleased;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void SetSimulateOnDrop(const bool bSimulate);
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void SetPrimitiveComponentPhysicsEnabled(const bool bSimulate);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		EGrabTypeBase GrabTypeBase;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void SnapToMotionController(UHeistMotionControllerComponent* MotionController, FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator);
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		EControllerHand GetHeldByHand(UHeistMotionControllerComponent* InMotionController = nullptr) const;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool IsBeingHeld() const;

};

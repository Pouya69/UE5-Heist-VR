// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HeistGrabComponent.generated.h"

class UPhysicsConstraintComponent;
// For custom, we can define our own Enums to be used.
UENUM(BlueprintType)
enum class EGrabTypeBase : uint8
{
	NONE,
	FREE,
	SNAP,
	TWO_HANDED,
	WEIGHTED_ONE_HANDED,
	CUSTOM
};


class UHeistMotionControllerComponent;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrabbed, UHeistGrabComponent*, GrabbedComponent, UHeistMotionControllerComponent*, MotionControllerRef);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReleased, UHeistGrabComponent*, GrabbbedComponent, UHeistMotionControllerComponent*, MotionControllerRef);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class METAXRTEST_01_API UHeistGrabComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHeistGrabComponent();
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		virtual void SetGrabbableVisible(const bool bIsGrabbableVisible);

protected:
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// HAS TO BE SET USING: InitializeGrabComponent()
	UPROPERTY(BlueprintReadOnly, Category="Grab Component | OtherComponents")
		TObjectPtr<UPrimitiveComponent> PrimitiveComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		bool bSimulateOnDrop;
	
	// We by default should be attaching it to the Physics Hands.
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool AttachPrimitiveCompTo(USceneComponent* AttachTo);
	
	UPROPERTY(BlueprintReadOnly, Category="Grab Component")
		bool bIsHeld;
	
	// For Two Handed Only
	UPROPERTY(BlueprintReadOnly, Category="Grab Component | Two Handed")
		bool bIsHeld_02;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		TObjectPtr<UHapticFeedbackEffect_Base> OnGrabHapticEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		TObjectPtr<USoundBase> OnGrabSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		TObjectPtr<UHapticFeedbackEffect_Base> OnReleaseHapticEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		TObjectPtr<USoundBase> OnReleaseSound;
	
public:
	UPROPERTY(BlueprintReadWrite, Category="Grab Component")
		UPhysicsConstraintComponent* PhysicsConstraintGrabbingThis;
	
	UPROPERTY(BlueprintReadWrite, Category="Grab Component | Two Handed")
		UPhysicsConstraintComponent* PhysicsConstraintGrabbingThis_02;
	
	UPROPERTY(BlueprintReadWrite, Category="Grab Component | OtherComponents")
		TObjectPtr<UHeistMotionControllerComponent> CurrentMotionControllerHoldingThis;
	
	// For Two Handed Only
	UPROPERTY(BlueprintReadWrite, Category="Grab Component | OtherComponents | Two Handed")
		TObjectPtr<UHeistMotionControllerComponent> CurrentMotionControllerHoldingThis_02;
	
	// No movement can be done. Only the hand gets stuck to the component.
	UFUNCTION(BlueprintCallable, Category="Grab Component | Two Handed")
		void AttachHandToGrabComponent(bool bAttach, UHeistMotionControllerComponent* MotionControllerRef);
	
	UPROPERTY(BlueprintReadOnly, Category="Grab Component | Two Handed")
		FVector GrabLocationCache;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void TraceFingersProcedural();
	
	// Call this in PostInitializeComponents() of the actor
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void InitializeGrabComponent(UPrimitiveComponent* InPrimitiveComp, const bool bWasInitializedFromActor = false);
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		UHeistMotionControllerComponent* GetCurrentMotionControllerHoldingThis();
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool IsGrabComponentReady() const;
	
	// Checks if bIsBeingHeld is true.
	UFUNCTION(BlueprintCallable, Category="Grab Component | Two Handed")
		bool IsReadyToGrab() const;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		virtual bool TryGrab(UHeistMotionControllerComponent* MotionController, USceneComponent* AttachTo, APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint = nullptr);
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		virtual bool TryRelease(UHeistMotionControllerComponent* MotionController, APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint = nullptr);
	
	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Grab Component")
		FOnGrabbed OnGrabbed;
	
	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Grab Component")
		FOnReleased OnReleased;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void SetSimulateOnDrop(const bool bSimulate);
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void SetPrimitiveComponentPhysicsEnabled(const bool bSimulate);
	
	UPrimitiveComponent* GetPrimitiveComponentAttached() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grab Component")
		EGrabTypeBase GrabTypeBase;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		void SnapTo(USceneComponent* AttachTo, FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator);
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		EControllerHand GetHeldByHand(UHeistMotionControllerComponent* InMotionController = nullptr) const;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool IsBeingHeld() const;

	// If Component Tick is enabled, when controller distance is too far from this, we detach it.
	UPROPERTY(EditAnywhere, Category="Grab Component")
		float HandDetachmentDistanceThreshold;
	
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		bool DetachWhenTooFarFromGrabbable();
};

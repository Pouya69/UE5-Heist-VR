// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Grabbable.h"
#include "Door.generated.h"

class UHeistMotionControllerComponent;
class UPhysicsConstraintComponent;

UCLASS()
class METAXRTEST_01_API ADoor : public AGrabbable
{
	GENERATED_BODY()

public:
	ADoor();

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<USceneComponent> HandleAnchorComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> DoorHandleComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> DoorMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UPhysicsConstraintComponent> DoorConstraint;
	
	UPROPERTY(BlueprintReadOnly, Category="Door | Handle")
		bool bIsHandlePushedDown;
	
	UPROPERTY(BlueprintReadOnly, Category="Door | Handle")
		bool bIsHandOnHandle;
	
	// When door's Yaw - the initial rotation (closed rotation) <= DoorClosedDifferenceYawThreshold -> Close door.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Door")
		float DoorClosedDifferenceYawThreshold;
	
	// How much minimum can the handle turn (relative)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Door | Handle")
		float DoorHandleMinPitch;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Door | Handle")
		FVector DoorHandleHandLocationOffset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Door | Handle")
		FRotator DoorHandleHandRotationOffset;
	
	// How much maximum can the handle turn (relative)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Door | Handle")
		float DoorHandleMaxPitch;
	
	// How much should handle rotate in pitch axis relative to trigger open.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Door | Handle")
		float DoorOpenTriggerHandleThresholdPitch;
	
	// If dist > , we will detach the hand ( TryRelease() )
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Door | Handle")
		float MaxDistanceBetweenPhysicsHandAndMotionController;
	
	// Door has to be closed at the start.
	FQuat DoorClosedRotation;
	
	virtual void PostInitializeComponents() override;
	
	UFUNCTION()
		void OnDoorHandleGrabbed(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION()
		void OnDoorHandleReleased(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	bool bIsDoorOpen = false;
	
	virtual void Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent) override;

public:
	UFUNCTION(BlueprintCallable, Category="Door")
		bool IsDoorOpen() const;
	
	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category="Door")
		bool CanDoorRotate() const;
	
	UFUNCTION(BlueprintCallable, Category="Door")
		bool CanDoorTick() const;
	
	virtual void Interact_Implementation() override;
	
	virtual void SetIsInteractable_Implementation(const bool bIsInteractable) override;
	
	UFUNCTION(BlueprintCallable, Category="Door")
		void OpenCloseDoor(bool bOpen);
};

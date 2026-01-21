// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Grabbable.h"
#include "HeistRemoteDetonator.generated.h"

class UHeistMotionControllerComponent;
class AHeistDynamite;

UCLASS(Abstract)
class METAXRTEST_01_API AHeistRemoteDetonator : public AGrabbable
{
	GENERATED_BODY()

public:
	AHeistRemoteDetonator();
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamite")
		TObjectPtr<AHeistDynamite> DynamiteLinked;

protected:
	// For when handle is at bottom and Tick() keeps ticking.
	UPROPERTY(BlueprintReadOnly, Category = "Detonate")
		bool bCanDetonate;
	
	virtual void Interact_Implementation() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Componenets")
		TObjectPtr<UHeistGrabComponent> HandleGrabComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Componenets")
		TObjectPtr<UStaticMeshComponent> HandleMeshComponent;
	
	virtual bool GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents) override;
	
	virtual void PostInitializeComponents() override;
	
	bool bIsInvertedForwardAndRight;
	
	FRotator MatchingRotationOffset;
	
	// The maximum the handle can go inside the detonator.
	UPROPERTY(EditDefaultsOnly, Category="Handle")
		float HandleMinimumHeight;
	
	// The maximum the handle can come out of the detonator.
	UPROPERTY(EditDefaultsOnly, Category="Handle")
		float HandleMaximumHeight;
	
	// The height when handle is not grabbed.
	UPROPERTY(EditDefaultsOnly, Category="Handle")
		float HandleIdleHeight;
	
	// The speed of transition of height when handle is not grabbed.
	UPROPERTY(EditDefaultsOnly, Category="Handle")
		float HandleIdleHeightTransitionSpeed;
	
	// When hand is locked on handle, what is the offset.
	UPROPERTY(EditDefaultsOnly, Category="VR Hands")
		FVector HandleHandGrabbingLocationOffset;
	
	// When hand is locked on handle, what is the offset.
	UPROPERTY(EditDefaultsOnly, Category="VR Hands")
		FRotator HandleHandGrabbingRotationOffset;
	
	UPROPERTY(EditDefaultsOnly, Category="VR Hands")
		float BasePivotOffsetZ;
	
	// After how long, the dynamite will explode. If <= 0, it will be instant.
	UPROPERTY(EditAnywhere, Category="Detonate")
		float DetonateAfterPushInSeconds;
	
	UPROPERTY(EditAnywhere, Category="Detonate")
		TObjectPtr<USoundBase> InteractSound;
	
	FTimerHandle DetonateTimerHandle;
	
	virtual void SetIsInteractable_Implementation(const bool bIsInteractable) override;
	
	UFUNCTION()
		void OnHandleGrabbed(UHeistGrabComponent* GrabComponentRef, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION()
		void OnHandleReleased(UHeistGrabComponent* GrabComponentRef, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION(BlueprintCallable, Category="Detonate")
		void Detonate();
	
	// If Dist > MaxDistanceBetweenControllerAndPhysicsHand, we stop holding the handle.
	UPROPERTY(EditDefaultsOnly, Category="VR Hands")
		float MaxDistanceBetweenControllerAndPhysicsHand;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	
};

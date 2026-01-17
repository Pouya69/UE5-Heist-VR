// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "HeistMotionControllerComponent.generated.h"


class UHeistGrabComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class METAXRTEST_01_API UHeistMotionControllerComponent : public UMotionControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHeistMotionControllerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	
	
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category="Components")
		void InitializeMotionControllerComponent(USkeletalMeshComponent* InPhysicsHand, USkeletalMeshComponent* InGhostHand);
	
	UFUNCTION(BlueprintCallable, Category="Components")
		bool IsMotionControllerReady() const;

	// Gets initialized using the InitializeMotionController()
	UPROPERTY(BlueprintReadOnly, Category="Components")
		TObjectPtr<USkeletalMeshComponent> PhysicsHandRef;
	
	// Gets initialized using the InitializeMotionController()
	UPROPERTY(BlueprintReadOnly, Category="Components")
		TObjectPtr<USkeletalMeshComponent> GhostHandRef;
	
	// For when we want to 'fake' hands' physics.
	UFUNCTION(BlueprintCallable, Category="Grab")
		void ConstrainTickGrab(float DeltaTime, FQuat GrabRotation, FVector GrabLocation = FVector::ZeroVector, bool bIsConstrained = false);
	
	UPROPERTY(BlueprintReadWrite)
		UHeistGrabComponent* CurrentGrabbedComp;
	
};

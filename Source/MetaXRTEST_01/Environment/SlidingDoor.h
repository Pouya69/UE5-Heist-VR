// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Grabbable.h"
#include "SlidingDoor.generated.h"

class UPhysicsConstraintComponent;

UCLASS()
class METAXRTEST_01_API ASlidingDoor : public AGrabbable
{
	GENERATED_BODY()

public:
	ASlidingDoor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UStaticMeshComponent> FirstHandleMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UStaticMeshComponent> SecondHandleMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UHeistGrabComponent> SecondHandleGrabComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UPhysicsConstraintComponent> DoorPhysicsConstraint;
	
	UPROPERTY(EditAnywhere, Category = "Hand Config")
		FVector HandLocationOffset;
	
	UPROPERTY(EditAnywhere, Category = "Hand Config")
		FRotator HandRotationOffset;
	
	UPROPERTY(EditAnywhere, Category = "Slide")
		float DoorSlideInterpSpeedToController;
	
	virtual void Custom_Tick_Implementation(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent) override;
	
	virtual void ReleaseConstraintFromAnchor_Implementation(UPhysicsConstraintComponent* ReleasedConstraintComp) override;
	
	FVector BaseLocationOffsetFromHandle;
	FVector BaseLocationOffsetFromHandle2;
	FVector InitialDoorLocation;
	
	UPROPERTY(EditInstanceOnly, Category = "Constraints")
		TObjectPtr<AActor> LinkedConstraintActor;
	
	UPROPERTY(BlueprintReadOnly, Category = "Constraints")
		TObjectPtr<UPhysicsConstraintComponent> LinkedConstraintComp;
	
	UFUNCTION()
		void OnHandleGrabbed(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION()
		void OnHandleReleased(UHeistGrabComponent* ReleasedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	virtual bool GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents) override;
	
	virtual bool IsGrabbable_Implementation(const FName BoneHit) const override;
	
	
	virtual void BeginPlay() override;
	
	virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaTime) override;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeistGrabComponent.h"
#include "HeistSkeletalGrabComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class METAXRTEST_01_API UHeistSkeletalGrabComponent : public UHeistGrabComponent
{
	GENERATED_BODY()

public:
	UHeistSkeletalGrabComponent();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BLueprintCallable, Category="Heist Skeletal Grab Component")
		bool IsGrabbableBasedOnBoneHit(const FName BoneHit) const;
	
		
	// Only if we hit these bones we grab. If empty, it will be grabbable everywhere.
	UPROPERTY(EditAnywhere, Category="Heist Skeletal Grab Component")
		TArray<FName> AcceptableBonesToGrab;
	
protected:
	virtual bool TryGrab(UHeistMotionControllerComponent* MotionController, USceneComponent* AttachTo, APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint = nullptr, const FVector& SnapGrabLocationOffset = FVector::ZeroVector) override;
	
	virtual bool TryRelease(UHeistMotionControllerComponent* MotionController, APlayerController* PlayerController, UPhysicsConstraintComponent* HandPhysicsConstraint = nullptr) override;
};

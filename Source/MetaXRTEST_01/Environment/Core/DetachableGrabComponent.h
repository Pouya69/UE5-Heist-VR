// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeistGrabComponent.h"
#include "DetachableGrabComponent.generated.h"


class AGrabbable;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class METAXRTEST_01_API UDetachableGrabComponent : public UHeistGrabComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDetachableGrabComponent();
	
	void AttachToAnchorPoint(AGrabbable* OwnerGrabbable, USceneComponent* NewAnchorToAttachTo, UHeistMotionControllerComponent* MotionController);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	
	virtual bool DetachWhenTooFarFromGrabbable() override;
	
	UPROPERTY(EditAnywhere, Category="Detach")
		bool bIsDetachable;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};

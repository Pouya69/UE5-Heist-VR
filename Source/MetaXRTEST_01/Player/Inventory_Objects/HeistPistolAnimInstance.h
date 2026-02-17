// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HeistPistolAnimInstance.generated.h"


UCLASS(Abstract)
class METAXRTEST_01_API UHeistPistolAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Pistol")
		void PistolShot();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Pistol")
		void PistolTriggerReleased();
	
	UPROPERTY(BlueprintReadWrite, Category="Pistol")
		float PistolTriggerAlpha;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CourierCharacter.generated.h"

UCLASS()
class METAXRTEST_01_API ACourierCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACourierCharacter();
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="References")
		APawn* PlayerPawnRef;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="References")
		USceneComponent* PlayerCameraRefSceneComponent;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="References")
		USceneComponent* CurrentLookingAtTarget;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="IK | Player")
		float LookAtPlayerAlpha;
	
	UFUNCTION(BlueprintCallable, Category = "Focus")
		void FocusOnPlayer();
	
	UFUNCTION(BlueprintCallable, Category = "Focus")
		void ClearFocus();

	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};

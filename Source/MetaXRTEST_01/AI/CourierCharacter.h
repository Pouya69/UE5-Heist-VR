// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CourierCharacter.generated.h"

class ACourierController;
class UMotionWarpingComponent;

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
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Interp, Category="IK | Player")
		float LookAtPlayerAlpha;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Interp, Category="IK | Player")
		float CutsceneAlpha;
	
	UFUNCTION(BlueprintCallable, Category = "IK | Player")
		void SetCutsceneAlpha(float NewAlpha);
	
	UFUNCTION(BlueprintCallable, Category = "IK | Player")
		void SetLookAtPlayerAlpha(float NewAlpha);
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Controller")
		TObjectPtr<ACourierController> CourierControllerRef;
	
	UFUNCTION(BlueprintCallable, Category = "Focus")
		void FocusOnPlayer(const float NewAlpha);
	
	UFUNCTION(BlueprintCallable, Category = "Focus")
		void ClearFocus();
	
	UFUNCTION(BlueprintCallable, Category = "Obstacle")
		void StartAttackObstacle();
	
	UFUNCTION(BlueprintCallable, Category = "Obstacle")
		void AttackObstacle();
	
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UMotionWarpingComponent> MotionWarpComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Obstacle")
		TObjectPtr<UAnimMontage> AttackObstacleMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Obstacle")
		FName ObstacleAttackForceBone;
	
	UPROPERTY(EditDefaultsOnly, Category = "Obstacle")
		float ObstacleCheckRadius;
	
	UPROPERTY(EditDefaultsOnly, Category = "Obstacle")
		float ObstaclePushAwayForce;
	
	virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};

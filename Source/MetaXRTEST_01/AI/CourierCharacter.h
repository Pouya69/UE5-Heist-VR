// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/HeistInteractionInterface.h"
#include "GameFramework/Character.h"
#include "CourierCharacter.generated.h"

class ALevelSequenceActor;
class ACourierController;
class UMotionWarpingComponent;

UCLASS()
class METAXRTEST_01_API ACourierCharacter : public ACharacter, public IHeistInteractionInterface
{
	GENERATED_BODY()

public:
	// For things that are controlled in the State Machine and not just Anim Montages or LSQs
	// By default it should be true for the start for when courier is in hallway door.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="References")
		bool bIsInDefaultCutsceneActions;
	
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
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Cutscenes")
		void InterruptedCutscene(const bool bEndCurrentCutscene = false);
	
	UPROPERTY(BlueprintReadWrite, Category="Cutscenes")
		ALevelSequenceActor* CurrentCutscenePlaying;
	
	void InterruptedCutscene_Implementation(const bool bEndCurrentCutscene = false);
	
	UFUNCTION(BlueprintCallable, Category="Cutscenes")
		void Sprint(const bool bIsSprinting = true);
	
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
	
	UPROPERTY(BlueprintReadWrite, Category="Size")
		EHeistSize CurrentSize;
	
protected:
	
	virtual void ObjectSlowedDown_Implementation() override;
	virtual void ObjectOutOfSlowMotion_Implementation() override;
	virtual bool IsGrabbable_Implementation(const FName BoneHit) const override;
	virtual bool IsRemoteGrabbable_Implementation() const override;
	virtual EHeistSize GetCurrentSizeOfGameObject_Implementation() override;
	
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

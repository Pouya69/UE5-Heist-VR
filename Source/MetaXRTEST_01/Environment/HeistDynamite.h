// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grabbable.h"
#include "HeistDynamite.generated.h"

class USphereComponent;
class UNiagaraSystem;

UCLASS(Abstract)
class METAXRTEST_01_API AHeistDynamite : public AGrabbable
{
	GENERATED_BODY()

public:
	AHeistDynamite();

protected:
	
	FTimerHandle ExplosionTimerHandle;
	
	virtual void PostInitializeComponents() override;
	
	UFUNCTION(BlueprintCallable, Category = "HeistDynamite")
		void Exploded(AActor* DestroyedActor);
	
	UFUNCTION(BlueprintCallable, Category = "HeistDynamite")
		void Explode();
	
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		TObjectPtr<UNiagaraSystem> ExplosionFX;
	
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
		TObjectPtr<USoundBase> ExplosionSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Explosion")
		float ExplosionTimerInSeconds;
	
	UPROPERTY(EditDefaultsOnly, Category = "Lock")
		TObjectPtr<USoundBase> LockPositionSound;
	
	void StartExplosion_Implementation();
	
	virtual void SetIsInteractable_Implementation(const bool bIsInteractable) override;
	virtual bool GetIsInteractable_Implementation() const override;
	virtual void Interact_Implementation() override;
	virtual void SetIsInFocus_Implementation(const bool bIsInFocus) override;
	
	virtual void ReleasedOnObjective_Implementation(AActor* ObjectiveActor) override;

	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Explosion")
		void StartExplosion();
	
	UFUNCTION(BlueprintCallable, Category="Dynamite")
		void LockToPosition(FVector Position);
	
};

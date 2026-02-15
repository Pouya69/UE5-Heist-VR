// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeistPistol.generated.h"

class UNiagaraSystem;
class AHeistBullet;

UCLASS(Abstract)
class METAXRTEST_01_API AHeistPistol : public AActor
{
	GENERATED_BODY()

public:
	AHeistPistol();
	
	void CustomPistolTick(const float Alpha);
	
	UFUNCTION(BlueprintCallable, Category = "Pistol")
		void TogglePistolEnabled(const bool bEnabled);
	
	UPROPERTY(BlueprintReadOnly, Category="Pistol")
		bool bIsPistolEnabled;
	
	UPROPERTY(VisibleAnywhere, Category="Pistol")
		FName PistolMuzzleSocketName;
	
	UFUNCTION(BlueprintCallable, Category="Pistol")
		bool ShootPistol();
	
	UFUNCTION(BlueprintCallable, Category="Pistol | Pooling")
		void InitializeBulletPools();
	
	void ResetBulletAndAddBackToPool(AHeistBullet* Bullet);
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Pistol | Animation")
		float PistolTriggerAlpha;
	
	USkeletalMeshComponent* GetPistolSkeletalMeshComponent();
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<USkeletalMeshComponent> PistolMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Pistol")
		TSubclassOf<AHeistBullet> BulletClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Shooting")
		TObjectPtr<USoundBase> MuzzleShootSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Shooting")
		TObjectPtr<UNiagaraSystem> MuzzleShootFX;
	
	TQueue<AHeistBullet*> NotActivePool;
	
	// The general pool amount
	UPROPERTY(BlueprintReadOnly, Category="Pistol | Pooling")
		int StartingAmountOfBulletsInPool;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Pistol | Time Dilation")
		float TargetTimeDilation;
	
	virtual void PostInitializeComponents() override;
};

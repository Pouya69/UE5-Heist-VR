

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeistBullet.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;
class AHeistPistol;
class UAudioComponent;

UCLASS(Abstract)
class METAXRTEST_01_API AHeistBullet : public AActor
{
	GENERATED_BODY()

public:
	AHeistBullet();
	
	UFUNCTION(BlueprintCallable, Category = "Bullet")
		void InitializeBullet(const FTransform& BulletTransform, const float BulletTargetTimeDilation);
	
	void InitializeBullet_FirstTime(AHeistPistol* InHeistPistol);
	
	UFUNCTION(BlueprintCallable, Category = "Bullet | Pooling")
		void AddBulletToPool();
	
	
	
	bool bIsActiveBullet;
	
	virtual void Tick(float DeltaSeconds) override;

protected:
	FTimerHandle DestroyTimerHandle;
	
	UPROPERTY(BlueprintReadOnly, Category="Bullet | Pooling")
		float DestroyBulletAfterSecondsAutomatically;
	
	UPROPERTY(BlueprintReadOnly, Category="Bullet | Pooling")
		AHeistPistol* PistolReference;
	
	UPROPERTY(BlueprintReadOnly, Category = "Bullet")
		float TargetTimeDilation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet")
		float TargetTimeDilationDuration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<USphereComponent> BulletSphereComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UAudioComponent> BulletLoopedAudioComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UNiagaraComponent> BulletLoopedFX;
	
	UPROPERTY(EditDefaultsOnly, Category="Shooting")
		TObjectPtr<USoundBase> BulletImpactSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Shooting")
		TObjectPtr<UNiagaraSystem> BulletImpactFX;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UProjectileMovementComponent> BulletProjectileMovementComponent;
	
	virtual void PostInitializeComponents() override;
	
	UFUNCTION()
		void OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
public:
};

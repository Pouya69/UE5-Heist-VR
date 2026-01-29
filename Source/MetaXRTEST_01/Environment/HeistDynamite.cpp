// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistDynamite.h"

#include "Core/HeistGrabComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"


AHeistDynamite::AHeistDynamite()
{
	// PrimaryActorTick.bCanEverTick = true;
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComponent->SetupAttachment(BaseMeshComponent);
	RadialForceComponent->Radius = 100.f;
	RadialForceComponent->bImpulseVelChange = true;
	RadialForceComponent->ImpulseStrength = 5000.f;
	RadialForceComponent->DestructibleDamage = 80.f;
	RadialForceComponent->SetComponentTickEnabled(false);
	RadialForceComponent->SetAutoActivate(false);
	
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	GrabComponent->GrabTypeBase = EGrabTypeBase::FREE;
	
	ExplosionTimerInSeconds = 0.5f;
}

void AHeistDynamite::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (!GetWorld()->IsGameWorld()) return;
	
	OnDestroyed.AddDynamic(this, &AHeistDynamite::Exploded);
}

void AHeistDynamite::Exploded(AActor* DestroyedActor)
{
	const FVector EffectLocation = GetActorLocation();
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionFX, EffectLocation);
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExplosionSound, EffectLocation);
}

void AHeistDynamite::Explode()
{
	RadialForceComponent->Activate();
	RadialForceComponent->FireImpulse();
	
	Destroy();
}

void AHeistDynamite::SetIsInteractable_Implementation(const bool bIsInteractable)
{
	GrabComponent->GrabTypeBase = bIsInteractable ? EGrabTypeBase::SNAP : EGrabTypeBase::NONE;
}

bool AHeistDynamite::GetIsInteractable_Implementation() const
{
	return GrabComponent->GrabTypeBase != EGrabTypeBase::NONE;
}

void AHeistDynamite::StartExplosion_Implementation()
{
	if (GetWorldTimerManager().IsTimerActive(ExplosionTimerHandle)) return;
	
	if (FMath::IsNearlyZero(ExplosionTimerInSeconds))
	{
		// Instant Explosion when no timer.
		Explode();
		return;
	}
	
	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AHeistDynamite::Explode, ExplosionTimerInSeconds, false);
}

void AHeistDynamite::LockToPosition(FVector Position)
{
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), LockPositionSound, GetActorLocation());
}

bool AHeistDynamite::IsRemoteGrabbable_Implementation() const
{
	return true;
}

void AHeistDynamite::Interact_Implementation()
{
	StartExplosion();
}

void AHeistDynamite::SetIsInFocus_Implementation(const bool bIsInFocus)
{
	
}

void AHeistDynamite::ReleasedOnObjective_Implementation(AActor* ObjectiveActor)
{
	LockToPosition(ObjectiveActor->GetActorLocation());
	GrabComponent->SetPrimitiveComponentPhysicsEnabled(false);
}
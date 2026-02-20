


#include "HeistBullet.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Inventory_Objects/HeistPistol.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Core/HeistFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"


AHeistBullet::AHeistBullet()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	BulletSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BulletSphereComp"));
	BulletSphereComponent->SetSphereRadius(10.0f);
	BulletSphereComponent->SetCollisionProfileName("BulletPreset");
	SetRootComponent(BulletSphereComponent);
	
	BulletProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("BulletProjectileMovementComp");
	// BulletProjectileMovementComponent->bAutoActivate = false;
	BulletProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	BulletProjectileMovementComponent->Bounciness = 0.0f;
	BulletProjectileMovementComponent->Friction = 5.0f;
	BulletProjectileMovementComponent->bInterpMovement = true;
	BulletProjectileMovementComponent->InitialSpeed = 500.0f;
	BulletProjectileMovementComponent->MaxSpeed = 500.0f;
	BulletProjectileMovementComponent->Velocity = FVector(500.0f, 0.0f, 0.0f);
	BulletProjectileMovementComponent->SetComponentTickEnabled(false);
	
	BulletLoopedAudioComp = CreateDefaultSubobject<UAudioComponent>("BulletLoopedAudioComp");
	BulletLoopedAudioComp->bAutoActivate = false;
	BulletLoopedAudioComp->SetComponentTickEnabled(false);
	BulletLoopedAudioComp->SetupAttachment(BulletSphereComponent);
	
	BulletLoopedFX = CreateDefaultSubobject<UNiagaraComponent>("BulletLoopedFXComp");
	BulletLoopedFX->bAutoActivate = false;
	BulletLoopedFX->SetComponentTickEnabled(false);
	BulletLoopedFX->SetupAttachment(BulletSphereComponent);
	
	TargetTimeDilationDuration = 4.0f;
	
	DestroyBulletAfterSecondsAutomatically = 8.0f;
	
	InitialLifeSpan = 10.0f;
}

void AHeistBullet::InitializeBullet(const FTransform& BulletTransform, const float BulletTargetTimeDilation)
{
	CustomTimeDilation = 1.0f / UGameplayStatics::GetGlobalTimeDilation(GetWorld());  // Make it move independetly.
	
	SetActorTransform(BulletTransform);
	
	TargetTimeDilation = BulletTargetTimeDilation;
	bIsActiveBullet = true;
	
	SetActorTickEnabled(true);
	
	BulletSphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	BulletSphereComponent->SetVisibility(true);
	
	BulletLoopedAudioComp->Activate();
	BulletLoopedAudioComp->SetComponentTickEnabled(true);
	
	BulletLoopedFX->Activate();
	BulletLoopedFX->SetComponentTickEnabled(true);
	
	FTimerDelegate Delegate;
	Delegate.BindLambda([&, BulletTransform]()
	{
		BulletProjectileMovementComponent->SetComponentTickEnabled(true);
		BulletProjectileMovementComponent->Activate();
		BulletProjectileMovementComponent->Velocity = UKismetMathLibrary::GetForwardVector(BulletTransform.Rotator()) * BulletProjectileMovementComponent->InitialSpeed;
		BulletProjectileMovementComponent->UpdateComponentVelocity();
		BulletProjectileMovementComponent->bSimulationEnabled = true;
	});
	GetWorldTimerManager().SetTimerForNextTick(Delegate);
	
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AHeistBullet::AddBulletToPool, DestroyBulletAfterSecondsAutomatically, false);
}

void AHeistBullet::InitializeBullet_FirstTime(AHeistPistol* InHeistPistol)
{
	BulletSphereComponent->IgnoreActorWhenMoving(PistolReference, true);
	PistolReference = InHeistPistol;
	
	PistolReference->GetPistolSkeletalMeshComponent()->IgnoreActorWhenMoving(this, true);
	
	AddBulletToPool();
}

void AHeistBullet::AddBulletToPool()
{
	GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
	
	SetActorTickEnabled(false);
	
	BulletSphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	BulletSphereComponent->SetVisibility(false);
	
	BulletProjectileMovementComponent->Deactivate();
	BulletProjectileMovementComponent->SetComponentTickEnabled(false);
	FHitResult HitResult;
	BulletProjectileMovementComponent->StopSimulating(HitResult);
	BulletProjectileMovementComponent->StopMovementImmediately();
	
	BulletLoopedAudioComp->Deactivate();
	BulletLoopedAudioComp->SetComponentTickEnabled(false);
	
	BulletLoopedFX->Deactivate();
	BulletLoopedFX->SetComponentTickEnabled(false);
	
	SetActorLocation(FVector(-100000000.0f,-100000000.0f, -100000000.0f));
	
	PistolReference->ResetBulletAndAddBackToPool(this);
	
	bIsActiveBullet = false;
	
	CustomTimeDilation = 1.0f;
}

void AHeistBullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	
}

void AHeistBullet::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	BulletSphereComponent->OnComponentHit.AddDynamic(this, &AHeistBullet::OnBulletHit);
	BulletSphereComponent->IgnoreActorWhenMoving(GetInstigator(), true);
}

void AHeistBullet::OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!PistolReference || !bIsActiveBullet) return;
	
	AddBulletToPool();
	
	const FVector EffectLocation = Hit.ImpactPoint;
	const FRotator EffectRotation = Hit.ImpactNormal.Rotation();
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BulletImpactFX, Hit.ImpactPoint, EffectRotation);
	UGameplayStatics::SpawnSoundAtLocation(this, BulletImpactSound, EffectLocation, EffectRotation);
	
	FTimerHandle TimerHandle;
	UHeistFunctionLibrary::SetTimeDilationOfObject(TimerHandle, OtherActor, TargetTimeDilation, TargetTimeDilationDuration);
	
	// OtherActor->CustomTimeDilation = TargetTimeDilation;
}

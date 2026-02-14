


#include "HeistBullet.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Inventory_Objects/HeistPistol.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"


AHeistBullet::AHeistBullet()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	BulletSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BulletSphereComp"));
	BulletSphereComponent->SetSphereRadius(10.0f);
	BulletSphereComponent->SetCollisionProfileName("BulletPreset");
	SetRootComponent(BulletSphereComponent);
	
	BulletProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("BulletProjectileMovementComp");
	BulletProjectileMovementComponent->bAutoActivate = false;
	BulletProjectileMovementComponent->SetComponentTickEnabled(false);
	
	BulletLoopedAudioComp = CreateDefaultSubobject<UAudioComponent>("BulletLoopedAudioComp");
	BulletLoopedAudioComp->bAutoActivate = false;
	BulletLoopedAudioComp->SetComponentTickEnabled(false);
	BulletLoopedAudioComp->SetupAttachment(BulletSphereComponent);
	
	BulletLoopedFX = CreateDefaultSubobject<UNiagaraComponent>("BulletLoopedFXComp");
	BulletLoopedFX->bAutoActivate = false;
	BulletLoopedFX->SetComponentTickEnabled(false);
	BulletLoopedFX->SetupAttachment(BulletSphereComponent);
}

void AHeistBullet::InitializeBullet(const FTransform& BulletTransform, const float BulletTargetTimeDilation)
{
	CustomTimeDilation = 1.0f / UGameplayStatics::GetGlobalTimeDilation(GetWorld());  // Make it move independetly.
	TargetTimeDilation = BulletTargetTimeDilation;
	bIsActiveBullet = true;
	
	SetActorTickEnabled(true);
	
	BulletSphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	BulletSphereComponent->SetVisibility(true);
	
	BulletProjectileMovementComponent->Activate();
	BulletProjectileMovementComponent->SetComponentTickEnabled(true);
	
	BulletLoopedAudioComp->Activate();
	BulletLoopedAudioComp->SetComponentTickEnabled(true);
	
	BulletLoopedFX->Activate();
	BulletLoopedFX->SetComponentTickEnabled(true);
}

void AHeistBullet::InitializeBullet_FirstTime(AHeistPistol* InHeistPistol)
{
	BulletSphereComponent->IgnoreActorWhenMoving(PistolReference, true);
	PistolReference = InHeistPistol;
	
	AddBulletToPool();
}

void AHeistBullet::AddBulletToPool()
{
	SetActorTickEnabled(false);
	
	BulletSphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	BulletSphereComponent->SetVisibility(false);
	
	BulletProjectileMovementComponent->Deactivate();
	BulletProjectileMovementComponent->SetComponentTickEnabled(false);
	
	BulletLoopedAudioComp->Deactivate();
	BulletLoopedAudioComp->SetComponentTickEnabled(false);
	
	BulletLoopedFX->Deactivate();
	BulletLoopedFX->SetComponentTickEnabled(false);
	
	SetActorLocation(FVector(-100000000.0f,-100000000.0f, -100000000.0f));
	
	PistolReference->ResetBulletAndAddBackToPool(this);
	
	bIsActiveBullet = false;
}

void AHeistBullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	CustomTimeDilation = 1.0f / UGameplayStatics::GetGlobalTimeDilation(this);
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
	AddBulletToPool();
	
	const FVector EffectLocation = Hit.ImpactPoint;
	const FRotator EffectRotation = Hit.ImpactNormal.Rotation();
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BulletImpactFX, Hit.ImpactPoint, EffectRotation);
	UGameplayStatics::SpawnSoundAtLocation(this, BulletImpactSound, EffectLocation, EffectRotation);
	
	OtherActor->CustomTimeDilation = TargetTimeDilation;
}

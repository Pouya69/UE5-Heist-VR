

#include "HeistPistol.h"

#include "NiagaraFunctionLibrary.h"
#include "Environment/HeistBullet.h"
#include "Kismet/GameplayStatics.h"


AHeistPistol::AHeistPistol()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	PistolMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("PistolMeshComp");
	PistolMeshComponent->SetCollisionProfileName("Hand_Equipment");
	SetRootComponent(PistolMeshComponent);
	
	PistolMuzzleSocketName = "Muzzle";
	StartingAmountOfBulletsInPool = 25;
}

void AHeistPistol::CustomPistolTick(const float Alpha)
{
	PistolTriggerAlpha = Alpha;
}

void AHeistPistol::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AHeistPistol::TogglePistolEnabled(const bool bEnabled)
{
	bIsPistolEnabled = bEnabled;
	PistolMeshComponent->SetVisibility(bEnabled);
	
	if (bEnabled)
	{
		PistolMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	}
	else
	{
		PistolMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);	
	}
}

bool AHeistPistol::ShootPistol()
{
	if (!bIsPistolEnabled) return false;
	
	const FTransform BulletSpawnTransform = PistolMeshComponent->GetSocketTransform(PistolMuzzleSocketName);
	
	AHeistBullet* BulletFromPool;
	const bool bDidGrabFromPool = NotActivePool.Dequeue(BulletFromPool);
	ensure(bDidGrabFromPool);
	
	if (!bDidGrabFromPool)
	{
		UE_LOG(LogTemp, Error, TEXT("Bullet Pool is Empty..."));
		return false;
	}
	
	const FVector EffectLocation = BulletSpawnTransform.GetTranslation();
	const FRotator EffectRotation = BulletSpawnTransform.GetRotation().Rotator();
	
	UGameplayStatics::SpawnSoundAtLocation(this, MuzzleShootSound, EffectLocation, EffectRotation);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MuzzleShootFX, EffectLocation, EffectRotation);
	
	BulletFromPool->InitializeBullet(BulletSpawnTransform, TargetTimeDilation);
	
	return true;
}

void AHeistPistol::InitializeBulletPools()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	for (int i = 0; i < StartingAmountOfBulletsInPool; ++i)
	{
		AHeistBullet* BulletSpawned = GetWorld()->SpawnActor<AHeistBullet>(BulletClass, FTransform::Identity, SpawnParams);
		BulletSpawned->InitializeBullet_FirstTime(this);
	}
}

void AHeistPistol::ResetBulletAndAddBackToPool(AHeistBullet* Bullet)
{
	NotActivePool.Enqueue(Bullet);
}

USkeletalMeshComponent* AHeistPistol::GetPistolSkeletalMeshComponent()
{
	return PistolMeshComponent;
}

void AHeistPistol::BeginPlay()
{
	Super::BeginPlay();
	
	// InitializeBulletPools();
}



#include "ObjectSizeChange.h"

#include "Components/SphereComponent.h"
#include "Core/HeistFunctionLibrary.h"
#include "Core/HeistGameMode.h"
#include "Core/HeistGrabComponent.h"
#include "Core/HeistTypes.h"
#include "Environment/Core/Grabbable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AObjectSizeChange::AObjectSizeChange()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComp"));
	SetRootComponent(BaseMeshComponent);
	BaseMeshComponent->SetCollisionProfileName("WorldStatic");
	BaseMeshComponent->SetCanEverAffectNavigation(false);
	
	ObjectOverlapSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ObjectOverlapSphereComp"));
	ObjectOverlapSphereComponent->SetCollisionProfileName("Size_Changer_Vaccum");
	ObjectOverlapSphereComponent->SetupAttachment(BaseMeshComponent);
	ObjectOverlapSphereComponent->SetGenerateOverlapEvents(true);
	
	SpitOutDirectionComponent = CreateDefaultSubobject<USceneComponent>("SpitOutDirectionComp");
	SpitOutDirectionComponent->SetupAttachment(BaseMeshComponent);
	
	AdditionToSpitTransform = 200.0f;
	
	MinForceOnSpitOut = 60.0f;
	MaxForceOnSpitOut = 100.0f;
}

void AObjectSizeChange::SpitOutObject(AGrabbable* GrabbableToSpitOut)
{
	if (CurrentSize == GrabbableToSpitOut->CurrentSize)
	{
		return;
	}
	
	GrabbableToSpitOut->ForceRelease();
	
	UPrimitiveComponent* ObjectPrimitiveComp = GrabbableToSpitOut->GetMainPrimitiveComponent();
	float NewObjectVelocityLength = ObjectPrimitiveComp->GetComponentVelocity().Length();
	FVector NewObjectAngularVelocity = ObjectPrimitiveComp->GetPhysicsAngularVelocityInRadians();
	
	const FVector Direction = UKismetMathLibrary::GetUpVector(SpitOutTransform.Rotator());
	FVector LocationAddition = Direction;
	
	const float Multiplier = UHeistFunctionLibrary::GetSizeMultiplierBasedOnType(CurrentSize);
	
	switch (CurrentSize)
	{
		default:
			break;
			
		case EHeistSize::TINY:
			NewObjectAngularVelocity *= Multiplier;
			NewObjectVelocityLength *= Multiplier;
			LocationAddition *= AdditionToSpitTransform / 1000;
			break;
		
		case EHeistSize::MEDIUM:
			NewObjectAngularVelocity *= Multiplier;
			NewObjectVelocityLength *= Multiplier;
			LocationAddition *= AdditionToSpitTransform;
			break;
	}
	
	UHeistFunctionLibrary::ChangeSizeTo(GrabbableToSpitOut, CurrentSize, FVector::ZeroVector, FRotator::ZeroRotator);
	
	GrabbableToSpitOut->ToggleActivateGrabbable(false);
	
	// ObjectPrimitiveComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
	ObjectPrimitiveComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	
	GrabbableToSpitOut->SetActorLocationAndRotation(SpitOutTransform.GetTranslation() + LocationAddition, GrabbableToSpitOut->GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);
	if (CurrentSize == EHeistSize::TINY)
	{
		ObjectPrimitiveComp->SetSimulatePhysics(false);
		ObjectPrimitiveComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ObjectPrimitiveComp->SetVisibility(false);
	}
	
	if (ObjectPrimitiveComp->IsSimulatingPhysics())
	{
		NewObjectVelocityLength = FMath::Clamp(NewObjectVelocityLength, MinForceOnSpitOut * Multiplier, MaxForceOnSpitOut * Multiplier);
	
		const FVector NewObjectVelocity = NewObjectVelocityLength * Direction;
		
		ObjectPrimitiveComp->SetPhysicsLinearVelocity(NewObjectVelocity);
		
	}
}

void AObjectSizeChange::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	const bool bActive = NewPlayerSize == CurrentSize;
	if (CurrentSize == EHeistSize::TINY)
	{
		BaseMeshComponent->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		BaseMeshComponent->SetVisibility(bActive, true);
	}
	ObjectOverlapSphereComponent->SetGenerateOverlapEvents(bActive);
	
	const float Multiplier = UHeistFunctionLibrary::GetSizeMultiplierBasedOnType_CHANGE(NewPlayerSize);
	SpitOutTransform.SetTranslationAndScale3D(SpitOutTransform.GetTranslation() * Multiplier, SpitOutTransform.GetScale3D() * Multiplier);
	
	/*
	if (bActive)
	{
		FVector NewObjectVelocity = Multiplier * 500.0f * UKismetMathLibrary::GetUpVector(SpitOutTransform.Rotator());
		
	}
	*/
}

void AObjectSizeChange::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		ObjectOverlapSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AObjectSizeChange::OnObjectEnteredVaccum);
		AHeistGameMode* GM = Cast<AHeistGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		GM->OnPlayerChangeSize.AddDynamic(this, &AObjectSizeChange::OnPlayerChangeSize);
		SpitOutTransform = SpitOutDirectionComponent->GetComponentTransform();
		const FTransform TransformCopy = SpitOutTransform;
		SpitOutDirectionComponent->DestroyComponent();
		
		OnPlayerChangeSize(EHeistSize::MEDIUM);
		
		SpitOutTransform.SetTranslationAndScale3D(TransformCopy.GetTranslation(), TransformCopy.GetScale3D());
	}
}

void AObjectSizeChange::OnObjectEnteredVaccum(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGrabbable* GrabbableEnteredVaccum = Cast<AGrabbable>(OtherActor);
	if (!GrabbableEnteredVaccum || GrabbableEnteredVaccum->GetGrabType() == EGrabTypeBase::TWO_HANDED || !GrabbableEnteredVaccum->IsRemoteGrabbable || GrabbableEnteredVaccum->CurrentSize == OtherObjectSizeChangerSide->CurrentSize) return;
	UPrimitiveComponent* ObjectPrimitiveComp = GrabbableEnteredVaccum->GetMainPrimitiveComponent();
	if (ObjectPrimitiveComp->GetComponentVelocity().GetSafeNormal().Dot(UKismetMathLibrary::GetUpVector(SpitOutTransform.Rotator())) >= 0.97f)
	{
		// Is exiting not entering.
		return;
	}
	
	OtherObjectSizeChangerSide->SpitOutObject(GrabbableEnteredVaccum);
}

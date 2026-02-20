

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
	
}

void AObjectSizeChange::SpitOutObject(AGrabbable* GrabbableToSpitOut)
{
	UPrimitiveComponent* ObjectPrimitiveComp = GrabbableToSpitOut->GetMainPrimitiveComponent();
	float NewObjectVelocityLength = ObjectPrimitiveComp->GetComponentVelocity().Length();
	FVector NewObjectAngularVelocity = ObjectPrimitiveComp->GetPhysicsAngularVelocityInRadians();
	
	switch (CurrentSize)
	{
		default:
			break;
			
		case EHeistSize::TINY:
			NewObjectAngularVelocity *= 1000.0f;
			NewObjectVelocityLength *= 1000.0f;
			break;
		
		case EHeistSize::MEDIUM:
			NewObjectAngularVelocity *= 0.001f;
			NewObjectVelocityLength *= 0.001f;
			break;
	}
	
	UHeistFunctionLibrary::ChangeSizeTo(GrabbableToSpitOut, CurrentSize, FVector::ZeroVector, FRotator::ZeroRotator);
	GrabbableToSpitOut->SetActorLocationAndRotation(SpitOutTransform.GetTranslation(), ObjectPrimitiveComp->GetComponentRotation());
	
	ObjectPrimitiveComp->SetPhysicsAngularVelocityInRadians(NewObjectAngularVelocity);
	
	FVector NewObjectVelocity = NewObjectVelocityLength * UKismetMathLibrary::GetUpVector(SpitOutTransform.Rotator());
	if (NewObjectVelocityLength <= 20.0f)
	{
		NewObjectVelocity += NewObjectVelocity.GetSafeNormal() * 5000.0f;
	}
	
	ObjectPrimitiveComp->SetPhysicsLinearVelocity(NewObjectVelocity);
}

void AObjectSizeChange::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	const bool bActive = NewPlayerSize == CurrentSize;
	if (CurrentSize == EHeistSize::TINY)
		BaseMeshComponent->SetVisibility(bActive, true);
	BaseMeshComponent->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	ObjectOverlapSphereComponent->SetGenerateOverlapEvents(bActive);
	
	const float Multiplier = UHeistFunctionLibrary::GetSizeMultiplierBasedOnType_CHANGE(NewPlayerSize);
	SpitOutTransform.SetTranslationAndScale3D(SpitOutTransform.GetTranslation() * Multiplier, SpitOutTransform.GetScale3D() * Multiplier);
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

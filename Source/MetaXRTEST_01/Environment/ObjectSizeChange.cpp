

#include "ObjectSizeChange.h"

#include "HeistWheel.h"
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


void AObjectSizeChange::SpitOutGrabbableAfterRelease(AGrabbable* GrabbableToSpitOut)
{
	UPrimitiveComponent* ObjectPrimitiveComp = GrabbableToSpitOut->GetMainPrimitiveComponent();
	ObjectPrimitiveComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FVector NewScale = GrabbableToSpitOut->GetActorScale3D() * UHeistFunctionLibrary::GetSizeMultiplierBasedOnType(CurrentSize);
	
	IHeistInteractionInterface::Execute_SetNewSizeTo(GrabbableToSpitOut, CurrentSize);
	
	
	

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
	
	

	// UE_LOG(LogTemp, Log, TEXT("%s"), *NewScale.ToString());
	// UHeistFunctionLibrary::ChangeSizeTo(GrabbableToSpitOut, CurrentSize, FVector::ZeroVector, FRotator::ZeroRotator);
	
	GrabbableToSpitOut->ToggleActivateGrabbable(false);
	
	// ObjectPrimitiveComp->SetPhysicsLinearVelocity(FVector::ZeroVector);

	
	GrabbableToSpitOut->SetActorLocationAndRotation(SpitOutTransform.GetTranslation() + LocationAddition, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
	if (CurrentSize == EHeistSize::TINY)
	{
		ObjectPrimitiveComp->SetSimulatePhysics(false);
		ObjectPrimitiveComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ObjectPrimitiveComp->SetVisibility(false, true);
	}
	else if (CurrentSize == EHeistSize::MEDIUM)
	{
		ObjectPrimitiveComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ObjectPrimitiveComp->SetSimulatePhysics(true);
		ObjectPrimitiveComp->SetVisibility(true, true);
	}
	
	GrabbableToSpitOut->SetActorScale3D(NewScale);
	
	if (ObjectPrimitiveComp->IsSimulatingPhysics())
	{
			
		ObjectPrimitiveComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		ObjectPrimitiveComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
		
		NewObjectVelocityLength = FMath::Clamp(NewObjectVelocityLength, MinForceOnSpitOut * Multiplier, MaxForceOnSpitOut * Multiplier);
	
		const FVector NewObjectVelocity = NewObjectVelocityLength * Direction;
		
		if (AHeistWheel* Wheel = Cast<AHeistWheel>(GrabbableToSpitOut))
		{
			if (CurrentSize == EHeistSize::MEDIUM)
			{
				GrabbableToSpitOut->SetActorLocationAndRotation(SpitOutTransform.GetTranslation() + (LocationAddition * 10.0f), FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
				
				FTimerDelegate Delegate;
				Delegate.BindLambda([&, GrabbableToSpitOut, ObjectPrimitiveComp, NewObjectVelocity, LocationAddition]()
				{
					ObjectPrimitiveComp->SetPhysicsLinearVelocity(NewObjectVelocity);
					ObjectPrimitiveComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
				});
				GetWorldTimerManager().SetTimerForNextTick(Delegate);
			}
		}
		else
		{
			FTimerDelegate Delegate;
			Delegate.BindLambda([&, GrabbableToSpitOut, ObjectPrimitiveComp, NewObjectVelocity, LocationAddition]()
			{
				ObjectPrimitiveComp->SetPhysicsLinearVelocity(NewObjectVelocity / 5000.0f);
				ObjectPrimitiveComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
			});
			GetWorldTimerManager().SetTimerForNextTick(Delegate);
			
		}
		
		// ObjectPrimitiveComp->SetPhysicsLinearVelocity(NewObjectVelocity);
		// ObjectPrimitiveComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	}
	
	/*
	FTimerDelegate Delegate;
	Delegate.BindLambda([&, GrabbableToSpitOut, LocationAddition]()
	{
		GrabbableToSpitOut->TeleportTo(SpitOutTransform.GetTranslation() + LocationAddition, FRotator::ZeroRotatorw);
	});
	*/
}

void AObjectSizeChange::SpitOutObject(AGrabbable* GrabbableToSpitOut)
{
	if (CurrentSize == GrabbableToSpitOut->CurrentSize)
	{
		return;
	}
	

	GrabbableToSpitOut->ForceRelease();
	
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, TEXT("SpitOutGrabbableAfterRelease"), GrabbableToSpitOut);
	
	GetWorldTimerManager().SetTimerForNextTick(Delegate);
	
	
}

void AObjectSizeChange::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	const bool bActive = NewPlayerSize == CurrentSize;
	
	if (CurrentSize == EHeistSize::TINY)
	{
		BaseMeshComponent->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		BaseMeshComponent->SetVisibility(bActive, true);
	}
	else
	{
		BaseMeshComponent->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
	
	ObjectOverlapSphereComponent->SetGenerateOverlapEvents(false);
	FTimerHandle TimerHandle;
	FTimerDelegate Delegate;
	Delegate.BindLambda([&, bActive]()
	{
		ObjectOverlapSphereComponent->SetGenerateOverlapEvents(bActive);
		RecentPrimitiveComponent = nullptr;
	});
	GetWorldTimerManager().SetTimer(TimerHandle, Delegate, 0.2f, false);
	
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
	if (RecentPrimitiveComponent && OtherComp == RecentPrimitiveComponent) return;
	AGrabbable* RecentGrabbable = Cast<AGrabbable>(OtherActor);
	
	ObjectOverlapSphereComponent->SetGenerateOverlapEvents(false);
	FTimerHandle TimerHandle;
	FTimerDelegate Delegate;
	Delegate.BindLambda([&]()
	{
		ObjectOverlapSphereComponent->SetGenerateOverlapEvents(true);
	});
	GetWorldTimerManager().SetTimer(TimerHandle, Delegate, 0.2f, false);
	
	if (!RecentGrabbable || !RecentGrabbable->bCanChangeSize || RecentGrabbable->CurrentSize == OtherObjectSizeChangerSide->CurrentSize) return;
	UPrimitiveComponent* ObjectPrimitiveComp = RecentGrabbable->GetMainPrimitiveComponent();
	if (ObjectPrimitiveComp->GetComponentVelocity().GetSafeNormal().Dot(UKismetMathLibrary::GetUpVector(SpitOutTransform.Rotator())) >= 0.97f)
	{
		// Is exiting not entering.
		return;
	}
	
	
	OtherObjectSizeChangerSide->SpitOutObject(RecentGrabbable);
}




#include "HeistMagnetCrane.h"

#include "Components/BoxComponent.h"
#include "Core/HeistGameMode.h"
#include "Core/HeistTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/RadialForceComponent.h"


AHeistMagnetCrane::AHeistMagnetCrane()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RadialForceComponentSocketAttachment = "Bone_004_end";
	
	CraneSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("CraneSkeletalMeshComp");
	CraneSkeletalMeshComponent->SetCollisionProfileName("Crane");
	SetRootComponent(CraneSkeletalMeshComponent);
	
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>("RadialForceComp");
	RadialForceComponent->bAutoActivate = false;
	RadialForceComponent->SetupAttachment(CraneSkeletalMeshComponent, RadialForceComponentSocketAttachment);
	RadialForceComponent->ForceStrength = -900000.0f;
	RadialForceComponent->Radius = 400.0f;
	
	OverlapDetectionBoxComp = CreateDefaultSubobject<UBoxComponent>("OverlapDetectionBoxComp");
	OverlapDetectionBoxComp->SetupAttachment(CraneSkeletalMeshComponent, RadialForceComponentSocketAttachment);
	OverlapDetectionBoxComp->SetCollisionProfileName("Only_Metal_Overlap");
	
	MaxHeight = 150.0f;
	MinHeight = -100.0f;
	
	MinRotationYaw = -80.0f;
	MaxRotationYaw = 80.0f;
	
	StartingDistance = 80.0f;
}

void AHeistMagnetCrane::BeginPlay()
{
	Super::BeginPlay();
}

void AHeistMagnetCrane::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	IK_Location = GetActorLocation() + (-GetActorRightVector() * StartingDistance);
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		AHeistGameMode* GM = Cast<AHeistGameMode>(GetWorld()->GetAuthGameMode());
		GM->OnPlayerChangeSize.AddDynamic(this, &AHeistMagnetCrane::OnPlayerChangeSize);
		
		CurrentYawNormalized = 0.5f;
		CurrentHeightNormalized = 0.5f;
		
		OverlapDetectionBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AHeistMagnetCrane::OnObjectEnteredOverlap);
		OverlapDetectionBoxComp->OnComponentEndOverlap.AddDynamic(this, &AHeistMagnetCrane::OnObjectExitOverlap);
	}
}

void AHeistMagnetCrane::OnObjectEnteredOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CompsAttached.Add(OtherComp);
	OtherComp->SetSimulatePhysics(false);
	FHitResult HitResult;
	OtherActor->SetActorLocation(RadialForceComponent->GetComponentLocation(), true, &HitResult, ETeleportType::TeleportPhysics);
	OtherComp->AttachToComponent(RadialForceComponent, FAttachmentTransformRules::KeepWorldTransform);
}

void AHeistMagnetCrane::OnObjectExitOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

void AHeistMagnetCrane::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	const float Multiplier = NewPlayerSize == EHeistSize::TINY ? MEDIUM_SIZE_MULT : TINY_SIZE_MULT;
	const float Multiplier_Opposite = NewPlayerSize == EHeistSize::TINY ? TINY_SIZE_MULT : MEDIUM_SIZE_MULT;
	
	RadialForceComponent->Radius *= Multiplier;
	RadialForceComponent->ForceStrength *= Multiplier;
	StartingDistance *= Multiplier;
	IK_Location *= Multiplier;
	MaxHeight *= Multiplier;
	MinHeight *= Multiplier;
	
	
	
	const bool bIsActive = RadialForceComponent->IsActive();
	if (bIsActive)
	{
		if (!CompsAttached.IsEmpty())
		{
			FHitResult HitResult;
			for (UPrimitiveComponent* OverlappedComponent : CompsAttached)
			{
				AActor* OverlappedActor = OverlappedComponent->GetOwner();
				OverlappedActor->SetActorLocation(OverlappedActor->GetActorLocation() * Multiplier_Opposite, false, &HitResult, ETeleportType::TeleportPhysics);
				OverlappedComponent->AttachToComponent(RadialForceComponent, FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}
	
}

void AHeistMagnetCrane::Interact_Implementation()
{
	RadialForceComponent->ToggleActive();
	
	const bool bIsActive = RadialForceComponent->IsActive();
	
	TArray<UPrimitiveComponent*> OverlappedComponents;
	OverlapDetectionBoxComp->GetOverlappingComponents(OverlappedComponents);
	
	if (bIsActive)
	{
		FHitResult HitResult;
		CompsAttached.Empty();
		for (int i = 0; i < OverlappedComponents.Num(); i++)
		{
			UPrimitiveComponent* OverlappedComponent = OverlappedComponents[i];
			OverlappedComponent->SetSimulatePhysics(false);
			OverlappedComponent->GetOwner()->SetActorLocation(RadialForceComponent->GetComponentLocation(), true, &HitResult, ETeleportType::TeleportPhysics);
			OverlappedComponent->AttachToComponent(RadialForceComponent, FAttachmentTransformRules::KeepWorldTransform);
			CompsAttached.Add(OverlappedComponent);
		}
	}
	else
	{
		for (int i = 0; i < OverlappedComponents.Num(); i++)
		{
			UPrimitiveComponent* OverlappedComponent = OverlappedComponents[i];
			OverlappedComponent->SetSimulatePhysics(true);
			
			OverlappedComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		}
		CompsAttached.Empty();
	}
	
	OverlapDetectionBoxComp->SetGenerateOverlapEvents(bIsActive);
}

void AHeistMagnetCrane::Interact_Reset_Implementation()
{
	// RadialForceComponent->Deactivate();
}

void AHeistMagnetCrane::SetAmount_Implementation(const float NewAmount)
{
	// Rotation of crane on world Yaw
	
	const float Amount = FMath::Clamp(NewAmount, 0.0f, 1.0f);
	
	const FVector BaseLocation = GetActorLocation();
	
	const float AngleAmount = MinRotationYaw + ((MaxRotationYaw - MinRotationYaw) / (1.0f - 0.0f)) * (Amount - 0.0f);
	const FVector NewLocationRelative = BaseLocation + UKismetMathLibrary::RotateAngleAxis(-GetActorRightVector() * StartingDistance, AngleAmount, FVector(0.0f, 0.0f, 1.0f));
	
	IK_Location.X = NewLocationRelative.X;
	IK_Location.Y = NewLocationRelative.Y;
	CurrentYawNormalized = Amount;
}

void AHeistMagnetCrane::SetAmount_02_Implementation(const float NewAmount)
{
	// Crane up and down.
	
	const float Amount = FMath::Clamp(NewAmount, 0.0f, 1.0f);
	
	const FVector BaseLocation = GetActorLocation();
	
	float HeightAmountRelative = MinHeight + ((MaxHeight - MinHeight) / (1.0f - 0.0f)) * (Amount - 0.0f);
	FVector NewLocation = IK_Location - BaseLocation;
	NewLocation.Z = HeightAmountRelative;
	
	IK_Location.Z = (NewLocation + BaseLocation).Z;
	CurrentHeightNormalized = Amount;
}

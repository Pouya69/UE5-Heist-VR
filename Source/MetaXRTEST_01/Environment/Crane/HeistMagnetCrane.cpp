


#include "HeistMagnetCrane.h"

#include "PhysicsEngine/RadialForceComponent.h"


AHeistMagnetCrane::AHeistMagnetCrane()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RadialForceComponentSocketAttachment = "EDIT_ENDBONE_INCODE";
	
	CraneSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("CraneSkeletalMeshComp");
	SetRootComponent(CraneSkeletalMeshComponent);
	
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>("RadialForceComp");
	RadialForceComponent->bAutoActivate = false;
	RadialForceComponent->SetupAttachment(CraneSkeletalMeshComponent, RadialForceComponentSocketAttachment);
	RadialForceComponent->ForceStrength = -90000.0f;
	RadialForceComponent->Radius = 300.0f;
}

void AHeistMagnetCrane::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHeistMagnetCrane::Interact_Implementation()
{
	RadialForceComponent->Activate();
}

void AHeistMagnetCrane::Interact_Reset_Implementation()
{
	RadialForceComponent->Deactivate();
}

void AHeistMagnetCrane::SetAmount_Implementation(const float NewAmount)
{
	// Rotation of crane on world Yaw
}

void AHeistMagnetCrane::SetAmount_02_Implementation(const float NewAmount)
{
	// Crane up and down.
}

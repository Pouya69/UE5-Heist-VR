// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistButton.h"

#include "Components/SphereComponent.h"
#include "Core/HeistGameMode.h"
#include "Core/HeistTypes.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AHeistButton::AHeistButton()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	ButtonPushedInDifference = 3.0f;
	ButtonReturnToStartSpeed = 10.0f;
	ButtonPushInSpeed = 40.0f;
	ButtonActivationForceThreshold = 8.0f;
	
	ButtonBaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonBaseMeshComp"));
	SetRootComponent(ButtonBaseMeshComponent);
	ButtonBaseMeshComponent->SetSimulatePhysics(false);
	ButtonBaseMeshComponent->SetCollisionProfileName("WorldStatic");
	
	ButtonMovingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMovingMeshComp"));
	ButtonMovingMeshComponent->SetSimulatePhysics(false);
	ButtonMovingMeshComponent->SetCollisionProfileName("PhysicsActor");
	ButtonMovingMeshComponent->SetupAttachment(ButtonBaseMeshComponent);
	
	TriggerSphereOverlapComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ButtonTriggerComp"));
	TriggerSphereOverlapComponent->SetCollisionProfileName("Button_Trigger");
	TriggerSphereOverlapComponent->SetupAttachment(ButtonBaseMeshComponent);
	
	bIsPowered = true;
	MinimumMass = -1.0f;
}

void AHeistButton::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		AHeistGameMode* GM = Cast<AHeistGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		GM->OnPlayerChangeSize.AddDynamic(this, &AHeistButton::OnPlayerChangeSize);
		
		OnPlayerChangeSize(EHeistSize::MEDIUM);
		
		TriggerSphereOverlapComponent->IgnoreComponentWhenMoving(ButtonBaseMeshComponent, true);
		TriggerSphereOverlapComponent->IgnoreComponentWhenMoving(ButtonMovingMeshComponent, true);
		ButtonMovingMeshComponent->IgnoreComponentWhenMoving(TriggerSphereOverlapComponent, true);
		ButtonBaseMeshComponent->IgnoreComponentWhenMoving(TriggerSphereOverlapComponent, true);
		ButtonBaseMeshComponent->IgnoreComponentWhenMoving(ButtonMovingMeshComponent, true);
		ButtonMovingMeshComponent->IgnoreComponentWhenMoving(ButtonBaseMeshComponent, true);
	
		TriggerSphereOverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &AHeistButton::OnTriggerOverlap);
		TriggerSphereOverlapComponent->OnComponentEndOverlap.AddDynamic(this, &AHeistButton::OnTriggerEndOverlap);
	}
	
	
}

// Called when the game starts or when spawned
void AHeistButton::BeginPlay()
{
	Super::BeginPlay();
	
	StartingButtonPosition = ButtonMovingMeshComponent->GetRelativeLocation().Z;
}

void AHeistButton::OnPlayerChangeSize(EHeistSize NewPlayerSize)
{
	if (CurrentSize == EHeistSize::TINY)
	{
		const bool bActive = NewPlayerSize == CurrentSize;
		ButtonBaseMeshComponent->SetVisibility(bActive, true);
		TriggerSphereOverlapComponent->SetGenerateOverlapEvents(bActive);
	}
}

void AHeistButton::Interact_Implementation()
{
	if (LinkedActor)
		Execute_Interact(LinkedActor);
}

void AHeistButton::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsPowered) return;
	if (bIsButtonActive) return;
	
	const bool bIsPlayer = OtherActor->IsA(APawn::StaticClass());
	if (bOnlyPlayerCanActivate && !bIsPlayer)
	{
		return;
	}
	
	if (!bIsPlayer)
		if (OtherComp->GetMass() < MinimumMass) return;
	
	if (OtherActor->Implements<UHeistInteractionInterface>())
	{
		if (IHeistInteractionInterface::Execute_GetCurrentSizeOfGameObject(OtherActor) != CurrentSize) return;
	}
	
	// if (OtherComp->GetPhysicsLinearVelocity().Length() <= ButtonActivationForceThreshold) return;
	
	SetActorTickEnabled(true);
	
	Execute_Interact(this);
	bIsButtonActive = true;
}

void AHeistButton::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// ButtonMovingMeshComponent->SetSimulatePhysics(false);
	if (!bIsButtonActive) return;
	
	if (!OtherActor->IsA(APawn::StaticClass()))
		if (OtherComp->GetMass() < MinimumMass) return;
	
	TArray<UPrimitiveComponent*> OverlappedComponents;
	GetOverlappingComponents(OverlappedComponents);
	
	bool bCanDeactivate = true;
	for (const UPrimitiveComponent* OverlappedComp : OverlappedComponents)
	{
		if (OverlappedComp->GetOwner()->IsA(APawn::StaticClass()) || OverlappedComp->GetMass() >= MinimumMass)
		{
			bCanDeactivate = false;
			break;
		}
	}
	
	if (OverlappedComponents.IsEmpty() || bCanDeactivate)
	{
		bIsButtonActive = false;
	}
}



// Called every frame
void AHeistButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector ButtonPosition = ButtonMovingMeshComponent->GetRelativeLocation();
	
	if (bIsButtonActive)
	{
		ButtonPosition.Z = FMath::FInterpConstantTo(ButtonPosition.Z, StartingButtonPosition - ButtonPushedInDifference, DeltaTime, ButtonPushInSpeed);
	}
	
	else
	{
		if (FMath::IsNearlyEqual(ButtonPosition.Z, StartingButtonPosition))
		{
			// Reached the starting point
			ButtonAtStartingPoint();
			SetActorTickEnabled(false);
			bIsButtonActive = false;
			return;
		}
		
		ButtonPosition.Z = FMath::FInterpConstantTo(ButtonPosition.Z, StartingButtonPosition, DeltaTime, ButtonReturnToStartSpeed);
	}
	
	ButtonMovingMeshComponent->SetRelativeLocation(ButtonPosition);
}

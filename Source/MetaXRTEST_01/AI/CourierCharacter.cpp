

#include "CourierCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "Player/HeistPlayerInterface.h"


ACourierCharacter::ACourierCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACourierCharacter::FocusOnPlayer()
{
	CurrentLookingAtTarget = PlayerCameraRefSceneComponent;
	LookAtPlayerAlpha = 1.0f;
}

void ACourierCharacter::ClearFocus()
{
	CurrentLookingAtTarget = nullptr;
	LookAtPlayerAlpha = 0.0f;
}

void ACourierCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerPawnRef = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawnRef && PlayerPawnRef->Implements<UHeistPlayerInterface>())
		PlayerCameraRefSceneComponent = IHeistPlayerInterface::Execute_GetPlayerCameraReferenceSoft(PlayerPawnRef);
}

void ACourierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

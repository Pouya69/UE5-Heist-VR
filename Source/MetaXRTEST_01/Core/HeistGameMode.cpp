


#include "HeistGameMode.h"

#include "HeistTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeistPlayerInterface.h"

void AHeistGameMode::StartPlay()
{
	Super::StartPlay();
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	Normal_VRCharacterRef = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void AHeistGameMode::ChangePlayerSize_Test(int NewSize)
{
	IHeistPlayerInterface::Execute_ChangeSize(UGameplayStatics::GetPlayerPawn(this, 0), static_cast<EHeistSize>(NewSize), FVector::ZeroVector);
}

void AHeistGameMode::ChangePlayerSize_Implementation(EHeistSize NewPlayerSize)
{
	switch (NewPlayerSize)
	{
		case EHeistSize::TINY:
			break;
	}
	OnPlayerChangeSize.Broadcast(NewPlayerSize);
	
}




#include "HeistGameMode.h"

void AHeistGameMode::ChangePlayerSize_Implementation(EHeistSize NewPlayerSize)
{
	OnPlayerChangeSize.Broadcast(NewPlayerSize);
}

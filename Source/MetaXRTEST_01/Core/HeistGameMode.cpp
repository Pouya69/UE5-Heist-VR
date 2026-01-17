


#include "HeistGameMode.h"

void AHeistGameMode::ChangePlayerSize_Implementation(EHeistPlayerSize NewPlayerSize)
{
	OnPlayerChangeSize.Broadcast(NewPlayerSize);
}

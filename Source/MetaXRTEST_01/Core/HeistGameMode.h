

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeistGameMode.generated.h"

enum class EHeistPlayerSize : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerChangeSize, EHeistPlayerSize, NewPlayerSize);

UCLASS()
class METAXRTEST_01_API AHeistGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Gameplay | Player Size")
		void ChangePlayerSize(EHeistPlayerSize NewPlayerSize);
	virtual void ChangePlayerSize_Implementation(EHeistPlayerSize NewPlayerSize);	
	
	UPROPERTY(BlueprintAssignable, Category="Gameplay | Player Size")
		FOnPlayerChangeSize OnPlayerChangeSize;
	
protected:
	
};

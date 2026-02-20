

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeistGameMode.generated.h"

enum class EHeistSize : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerChangeSize, EHeistSize, NewPlayerSize);

UCLASS()
class METAXRTEST_01_API AHeistGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	virtual void StartPlay() override;
	
	UFUNCTION(Exec)
		void ChangePlayerSize_Test(int NewSize);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Gameplay | Player Size")
		void ChangePlayerSize(EHeistSize NewPlayerSize);
	virtual void ChangePlayerSize_Implementation(EHeistSize NewPlayerSize);	
	
	UPROPERTY(BlueprintAssignable, Category="Gameplay | Player Size")
		FOnPlayerChangeSize OnPlayerChangeSize;
	
	UPROPERTY(BlueprintReadWrite, Category="Gameplay | Player")
		TObjectPtr<APawn> Small_VRCharacterRef;
	
	UPROPERTY(BlueprintReadOnly, Category="Gameplay | Player")
		TObjectPtr<APawn> Normal_VRCharacterRef;
	
	UPROPERTY(BlueprintReadWrite, Category="Gameplay | Player")
		TObjectPtr<AActor> MasterScaling_ActorRef;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Gameplay | Player")
		TSubclassOf<APawn> Small_VRCharacterClass;

};

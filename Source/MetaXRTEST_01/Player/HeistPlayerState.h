// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HeistPlayerState.generated.h"

class UHeistPlayerMainComponent;
/**
 * 
 */
UCLASS(Abstract)
class METAXRTEST_01_API AHeistPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Player")
		TObjectPtr<UHeistPlayerMainComponent> HeistPlayerMainComponentRef;
	
	UPROPERTY(BlueprintReadOnly, Category = "Player")
		bool bHasPistol;
	
	UPROPERTY(BlueprintReadOnly, Category = "Player")
		bool bHasChangeSizeTool;
	
	UFUNCTION(BlueprintCallable, Category = "Player")
		void PickedUpPistol();
};

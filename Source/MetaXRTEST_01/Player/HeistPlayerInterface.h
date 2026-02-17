

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeistPlayerInterface.generated.h"

enum class EHeistSize : uint8;

UINTERFACE()
class UHeistPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class METAXRTEST_01_API IHeistPlayerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void TraceRightFinger();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void TraceLeftFinger();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface | Procedural Animation")
		bool GetProceduralFingersAnimData();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void SetupBonePhysicsAndWeightLeftHand_CPP(FName RootBoneName, bool bSimulate);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void SetupBonePhysicsAndWeightRightHand_CPP(FName RootBoneName, bool bSimulate);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		bool ChangeSize(EHeistSize PlayerSize);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PistolEquipped();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PistolUnequipped();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PistolTriggerHeldDown();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PistolTriggerReleased();
};

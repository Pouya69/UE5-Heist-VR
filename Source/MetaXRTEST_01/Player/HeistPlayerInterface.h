

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeistPlayerInterface.generated.h"

class UCameraComponent;
enum class EHeistEquipmentType : uint8;
struct FPlayerChangeSizeInfo;
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
		void SetupBonePhysicsAndWeightLeftHand_CPP(FName RootBoneName, bool bSimulate, const bool bAlsoAttachDetach = false);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void SetupBonePhysicsAndWeightRightHand_CPP(FName RootBoneName, bool bSimulate, const bool bAlsoAttachDetach = false);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void SetupBothHandsBonePhysicsAndWeightRightHand_CPP(bool bSimulate, const bool bAlsoAttachDetach = false);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		bool ChangeSize(EHeistSize PlayerSize, const FVector NewLocation);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void GotInCart(const bool bInCart);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void TeleportPlayerTo(const FVector NewLocation);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void LeftForceRelease();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void RightForceRelease();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void RightForceGrab();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void LeftForceGrab();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PossessOtherPlayer(const FVector& NewLocation, const FRotator& NewRotation, const FVector& NewCameraLocation, const FRotator& NewCameraRotation, const EHeistEquipmentType EquipmentInHand);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PistolEquipped();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PistolUnequipped();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PistolTriggerHeldDown();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void PistolTriggerReleased();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Interface")
		void OnPickedUpPistolForFirstTime();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Interface")
		USceneComponent* GetPlayerCameraReferenceSoft() const;
};

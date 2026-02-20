#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeistInteractionInterface.generated.h"


enum class EHeistSize : uint8;
enum class EHeistGrabHandState : uint8;
class UHeistGrabComponent;

UINTERFACE(MinimalAPI)
class UHeistInteractionInterface : public UInterface
{
	GENERATED_BODY()
};


class METAXRTEST_01_API IHeistInteractionInterface
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
    	void Interact();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void CannotInteract();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void SetIsInFocus(const bool bIsInFocus);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void OnCoolDown();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		bool GetGrabComponents(TArray<UHeistGrabComponent*>& OutGrabComponents);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		bool IsRemoteGrabbable() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		bool IsGrabbable(const FName BoneHit) const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		EHeistGrabHandState GetHandAnimationType() const;
	
	virtual bool IsGrabbable_Implementation(const FName BoneHit) const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		bool RemoteGrab();
	
	virtual bool IsRemoteGrabbable_Implementation() const;
	
	// This functionality is for when the hand is supposed to be locked in place and/or moving in a specific direction.
	// This will be called inside HeistPlayerMainComponent Tick() when something is grabbed.
	// We pass the grabcomp for objects that have multiple CUSTOM types.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void Custom_Tick(const float& DeltaTime, const UHeistGrabComponent* WhichGrabComponent);
	
	// Used for when object is not actually placed and the green / red outline is being showed.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void SetIsInPreviewMode(const bool bIsInFocus);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void SetIsInteractable(const bool bIsInteractable);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		bool GetIsInteractable() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void ReleasedOnObjective(AActor* ObjectiveActor);
	
	// Useful for things that are derrived from wheels etc.
	// Amount is Normalize (0 <-> 1)
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void SetAmount(const float NewAmount);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		EHeistSize GetCurrentSizeOfGameObject();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void SetNewSizeTo(EHeistSize NewSize);
};

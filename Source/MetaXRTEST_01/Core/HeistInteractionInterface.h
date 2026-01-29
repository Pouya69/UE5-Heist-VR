#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeistInteractionInterface.generated.h"


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
		bool RemoteGrab();
	
	virtual bool IsRemoteGrabbable_Implementation() const;
	
	
	
	// Used for when object is not actually placed and the green / red outline is being showed.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void SetIsInPreviewMode(const bool bIsInFocus);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void SetIsInteractable(const bool bIsInteractable);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		bool GetIsInteractable() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
		void ReleasedOnObjective(AActor* ObjectiveActor);
};

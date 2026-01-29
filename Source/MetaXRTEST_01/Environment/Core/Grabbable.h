// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/HeistInteractionInterface.h"
#include "GameFramework/Actor.h"
#include "Grabbable.generated.h"


class UHeistGrabComponent;

UCLASS()
class METAXRTEST_01_API AGrabbable : public AActor, public IHeistInteractionInterface
{
	GENERATED_BODY()

public:
	AGrabbable();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UStaticMeshComponent> BaseMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UHeistGrabComponent> GrabComponent;
	
	virtual void SetIsInFocus_Implementation(const bool bIsInFocus) override;
	
	virtual bool IsRemoteGrabbable_Implementation() const override;
	
	virtual bool RemoteGrab_Implementation() override;
	
	FTimerHandle RemoteGrabTimerHandle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials")
		TObjectPtr<UMaterialInterface> InFocusMaterial;
	
	virtual bool GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents) override;
};

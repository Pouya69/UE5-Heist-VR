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
	
	virtual bool GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents) override;
};

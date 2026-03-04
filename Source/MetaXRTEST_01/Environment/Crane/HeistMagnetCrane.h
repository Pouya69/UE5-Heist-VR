// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/HeistInteractionInterface.h"
#include "GameFramework/Actor.h"
#include "HeistMagnetCrane.generated.h"

class URadialForceComponent;

UCLASS(Abstract)
class METAXRTEST_01_API AHeistMagnetCrane : public AActor, public IHeistInteractionInterface
{
	GENERATED_BODY()

public:
	AHeistMagnetCrane();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<USkeletalMeshComponent> CraneSkeletalMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<URadialForceComponent> RadialForceComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
		FName RadialForceComponentSocketAttachment;
	
	virtual void Interact_Implementation() override;
	virtual void Interact_Reset_Implementation() override;
	virtual void SetAmount_Implementation(const float NewAmount) override;
	virtual void SetAmount_02_Implementation(const float NewAmount) override;
};

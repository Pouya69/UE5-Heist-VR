// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeistButton.generated.h"

UCLASS()
class METAXRTEST_01_API AHeistButton : public AActor
{
	GENERATED_BODY()

public:
	AHeistButton();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};

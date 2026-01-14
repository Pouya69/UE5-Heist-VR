// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeistFunctionLibrary.generated.h"

class UHeistGrabComponent;
/**
 * 
 */
UCLASS()
class METAXRTEST_01_API UHeistFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Grab Component")
		static bool GetGrabComponents(AActor* Actor, TArray<UHeistGrabComponent*>& OutGrabComponents);
};

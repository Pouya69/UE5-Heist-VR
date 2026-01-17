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
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Grab Component")
		static bool GetGrabComponents(AActor* Actor, TArray<UHeistGrabComponent*>& OutGrabComponents);
	
	// When timer is up, this will be true (Duration was > 0)
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Time")
		static bool IsGlobalTimeDilationTemporary(const FTimerHandle& GlobalTimeDilationTimerHandle, UWorld* World);
	
	// If Duration <= 0, it will be forever
	// Overrides whatever that was previously set.
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Time")
		static bool SetTimeDilationOfEveryone(FTimerHandle& GlobalTimeDilationTimerHandle, UWorld* World, const float DilationAmount, const float AudioPitchDilation = 0.5f, const float Duration = -1.0f);
		
		
};

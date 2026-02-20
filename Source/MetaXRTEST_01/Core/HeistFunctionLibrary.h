// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeistFunctionLibrary.generated.h"

enum class EHeistSize : uint8;
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
		static bool IsGlobalTimeDilationTemporary(const FTimerHandle& GlobalTimeDilationTimerHandle, AActor* WorldReference);
	
	// If Duration <= 0, it will be forever
	// Overrides whatever that was previously set.
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Time")
		static bool SetTimeDilationOfEveryone(FTimerHandle& GlobalTimeDilationTimerHandle, AActor* WorldReference, const float DilationAmount, const float AudioPitchDilation = 0.5f, const float Duration = -1.0f);
		
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Time")
		static bool SetTimeDilationOfObject(FTimerHandle& GlobalTimeDilationTimerHandle, AActor* ObjectToAffect, const float DilationAmount, const float Duration = -1.0f);
	
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Time")
		static bool RestoreTimeToNormal(AActor* WorldReference, UPARAM(ref) FTimerHandle& OutTimeDilationTimerHandle, const float AudioRestoreDuration = 1.f);
	
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Time")
		static bool RestoreTimeToNormalForObject(AActor* ObjectToAffect, UPARAM(ref) FTimerHandle& TimeDilationTimerHandle);
	
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Size")
		static bool ChangeSizeTo(AActor* ObjectToAffect, EHeistSize NewSize, const FVector NewLocation = FVector::ZeroVector, const FRotator NewRotation = FRotator::ZeroRotator);
	
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Size")
		static FVector GetNewSizeOfGameObject(AActor* ObjectToAffect, EHeistSize NewSize);
	
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Size")
		static FVector GetNewSizeOfComponent(USceneComponent* ComponentToAffect, EHeistSize NewSize, const bool bIsWorldSpace = true);
	
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Size")
		static float GetSizeMultiplierBasedOnType(EHeistSize NewSize);
	
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Size")
		static float GetSizeMultiplierBasedOnType_CHANGE(EHeistSize NewSize);
		
	UFUNCTION(BlueprintCallable, Category="Heist Function Library | Gameplay | Size")
		static float GetAbsoluteSizeBasedOnType(EHeistSize NewSize);
};

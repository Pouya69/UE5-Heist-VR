// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistFunctionLibrary.h"

#include "HeistInteractionInterface.h"
#include "Kismet/GameplayStatics.h"

bool UHeistFunctionLibrary::GetGrabComponents(AActor* Actor, TArray<UHeistGrabComponent*>& OutGrabComponents)
{
	if (!Actor || !Actor->Implements<UHeistInteractionInterface>()) return false;
	return IHeistInteractionInterface::Execute_GetGrabComponents(Actor, OutGrabComponents);
}

bool UHeistFunctionLibrary::IsGlobalTimeDilationTemporary(const FTimerHandle& GlobalTimeDilationTimerHandle, AActor* WorldReference)
{
	return WorldReference->GetWorld()->GetTimerManager().IsTimerActive(GlobalTimeDilationTimerHandle);
}

bool UHeistFunctionLibrary::SetTimeDilationOfEveryone(FTimerHandle& GlobalTimeDilationTimerHandle, AActor* WorldReference, const float DilationAmount, const float AudioPitchDilation, const float Duration)
{
	UWorld* ObjectWorld = WorldReference->GetWorld();
	ObjectWorld->GetTimerManager().ClearTimer(GlobalTimeDilationTimerHandle);
	
	// Instant
	UGameplayStatics::SetGlobalPitchModulation(ObjectWorld, AudioPitchDilation, 0.0f);
	UGameplayStatics::SetGlobalTimeDilation(ObjectWorld, DilationAmount);
	
	if (Duration > 0.0f)
	{
		FTimerHandle NewGlobalTimeDilationTimerHandle;
		// Timer based to go back to normal.
		
		FTimerDelegate Delegate;
		Delegate.BindLambda([ObjectWorld, Duration]()
		{
			UGameplayStatics::SetGlobalPitchModulation(ObjectWorld, 1.f, Duration / 2);
			UGameplayStatics::SetGlobalTimeDilation(ObjectWorld, 1.0f);
		});
		
		ObjectWorld->GetTimerManager().SetTimer(GlobalTimeDilationTimerHandle, Delegate, Duration, false);
		GlobalTimeDilationTimerHandle = NewGlobalTimeDilationTimerHandle;
	}
	return true;
}

bool UHeistFunctionLibrary::RestoreTimeToNormal(AActor* WorldReference, FTimerHandle& GlobalTimeDilationTimerHandle, const float AudioRestoreDuration)
{
	if (WorldReference == nullptr) return false;
	UWorld* World =  WorldReference->GetWorld();
	if (!World->GetTimerManager().IsTimerActive(GlobalTimeDilationTimerHandle)) return false;
	
	World->GetTimerManager().ClearTimer(GlobalTimeDilationTimerHandle);
	UGameplayStatics::SetGlobalPitchModulation(World, 1.f, AudioRestoreDuration / 2);
	UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
	
	return true;
}

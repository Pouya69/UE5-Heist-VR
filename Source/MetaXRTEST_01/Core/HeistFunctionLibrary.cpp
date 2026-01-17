// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistFunctionLibrary.h"

#include "HeistInteractionInterface.h"
#include "Kismet/GameplayStatics.h"

bool UHeistFunctionLibrary::GetGrabComponents(AActor* Actor, TArray<UHeistGrabComponent*>& OutGrabComponents)
{
	if (!Actor || !Actor->Implements<UHeistInteractionInterface>()) return false;
	return IHeistInteractionInterface::Execute_GetGrabComponents(Actor, OutGrabComponents);
}

bool UHeistFunctionLibrary::IsGlobalTimeDilationTemporary(const FTimerHandle& GlobalTimeDilationTimerHandle, UWorld* World)
{
	return World->GetTimerManager().IsTimerActive(GlobalTimeDilationTimerHandle);
}

bool UHeistFunctionLibrary::SetTimeDilationOfEveryone(FTimerHandle& GlobalTimeDilationTimerHandle, UWorld* World, const float DilationAmount, const float AudioPitchDilation, const float Duration)
{
	World->GetTimerManager().ClearTimer(GlobalTimeDilationTimerHandle);
	
	// Instant
	UGameplayStatics::SetGlobalPitchModulation(World, AudioPitchDilation, 0.0f);
	UGameplayStatics::SetGlobalTimeDilation(World, DilationAmount);
	
	if (Duration > 0.0f)
	{
		// Timer based to go back to normal.
		
		FTimerDelegate Delegate;
		Delegate.BindLambda([&]()
		{
			UGameplayStatics::SetGlobalPitchModulation(World, 1.f, Duration / 2);
			UGameplayStatics::SetGlobalTimeDilation(World, 0.0f);
		});
		
		World->GetTimerManager().SetTimer(GlobalTimeDilationTimerHandle, Delegate, Duration, false);
	}
	
	return true;
}

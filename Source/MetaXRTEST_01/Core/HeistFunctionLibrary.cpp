// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistFunctionLibrary.h"

#include "HeistInteractionInterface.h"
#include "HeistTypes.h"
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
	UGameplayStatics::SetGlobalPitchModulation(ObjectWorld, AudioPitchDilation, 0.2f);
	UGameplayStatics::SetGlobalTimeDilation(ObjectWorld, DilationAmount);
	UGameplayStatics::GetPlayerPawn(ObjectWorld, 0)->CustomTimeDilation = 1.0f / DilationAmount;
	
	if (Duration > 0.0f)
	{
		FTimerHandle NewGlobalTimeDilationTimerHandle;
		// Timer based to go back to normal.
		
		FTimerDelegate Delegate;
		Delegate.BindLambda([ObjectWorld, Duration]()
		{
			UGameplayStatics::SetGlobalPitchModulation(ObjectWorld, 10.f, Duration / 2);
			UGameplayStatics::SetGlobalTimeDilation(ObjectWorld, 1.0f);
			UGameplayStatics::GetPlayerPawn(ObjectWorld, 0)->CustomTimeDilation = 1.0f;
		});
		
		ObjectWorld->GetTimerManager().SetTimer(GlobalTimeDilationTimerHandle, Delegate, Duration, false);
		GlobalTimeDilationTimerHandle = NewGlobalTimeDilationTimerHandle;
	}
	return true;
}

bool UHeistFunctionLibrary::SetTimeDilationOfObject(FTimerHandle& OutTimeDilationTimerHandle, AActor* ObjectToAffect,
	const float DilationAmount, const float Duration)
{
	UWorld* ObjectWorld = ObjectToAffect->GetWorld();
	ObjectWorld->GetTimerManager().ClearTimer(OutTimeDilationTimerHandle);
	
	// Instant
	ObjectToAffect->CustomTimeDilation = DilationAmount;
	
	if (Duration > 0.0f)
	{
		FTimerHandle NewGlobalTimeDilationTimerHandle;
		// Timer based to go back to normal.
		
		FTimerDelegate Delegate;
		Delegate.BindLambda([ObjectWorld, Duration, ObjectToAffect]()
		{
			ObjectToAffect->CustomTimeDilation = 1.0f;
		});
		
		ObjectWorld->GetTimerManager().SetTimer(OutTimeDilationTimerHandle, Delegate, Duration, false);
		OutTimeDilationTimerHandle = NewGlobalTimeDilationTimerHandle;
	}
	return true;
}

bool UHeistFunctionLibrary::RestoreTimeToNormal(AActor* WorldReference, FTimerHandle& GlobalTimeDilationTimerHandle, const float AudioRestoreDuration)
{
	if (WorldReference == nullptr) return false;
	UWorld* World =  WorldReference->GetWorld();
	if (!World->GetTimerManager().IsTimerActive(GlobalTimeDilationTimerHandle)) return false;
	
	World->GetTimerManager().ClearTimer(GlobalTimeDilationTimerHandle);
	UGameplayStatics::GetPlayerPawn(World, 0)->CustomTimeDilation = 1.0f;
	UGameplayStatics::SetGlobalPitchModulation(World, 10.f, AudioRestoreDuration / 2);
	UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
	
	return true;
}

bool UHeistFunctionLibrary::RestoreTimeToNormalForObject(AActor* ObjectToAffect, FTimerHandle& TimeDilationTimerHandle)
{
	ObjectToAffect->CustomTimeDilation = 1.0f;
	ObjectToAffect->GetWorld()->GetTimerManager().ClearTimer(TimeDilationTimerHandle);
	
	return true;
}

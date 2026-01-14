// Fill out your copyright notice in the Description page of Project Settings.


#include "HeistFunctionLibrary.h"

#include "HeistInteractionInterface.h"

bool UHeistFunctionLibrary::GetGrabComponents(AActor* Actor, TArray<UHeistGrabComponent*>& OutGrabComponents)
{
	if (!Actor || !Actor->Implements<UHeistInteractionInterface>()) return false;
	return IHeistInteractionInterface::Execute_GetGrabComponents(Actor, OutGrabComponents);
}

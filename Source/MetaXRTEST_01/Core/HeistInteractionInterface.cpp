// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/HeistInteractionInterface.h"

bool IHeistInteractionInterface::IsGrabbable_Implementation(const FName BoneHit) const
{
	return true;
}

// Add default functionality here for any IHeistInteractionInterface functions that are not pure virtual.
bool IHeistInteractionInterface::IsRemoteGrabbable_Implementation() const
{
	return true;
}

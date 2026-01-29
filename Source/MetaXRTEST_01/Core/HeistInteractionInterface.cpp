// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/HeistInteractionInterface.h"

// Add default functionality here for any IHeistInteractionInterface functions that are not pure virtual.
bool IHeistInteractionInterface::IsRemoteGrabbable_Implementation() const
{
	return true;
}

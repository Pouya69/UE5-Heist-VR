#pragma once

#define GRAB_CHANNEL ECC_GameTraceChannel2
#define VR_WIDGET_CHANNEL ECC_GameTraceChannel1
#define BULLET_CHANNEL ECC_GameTraceChannel3

UENUM(BlueprintType)
enum class EHeistSize : uint8
{
	TINY,
	MEDIUM,
};

UENUM(BlueprintType)
enum class EHeistGrabHandState : uint8
{
	DEFAULT,
	PISTOL,
	NOB
};

UENUM(BlueprintType)
enum class EHeistObjectInteractionType : uint8
{
	TRIGGER_ON_INTERACTION_ONLY,
	CONTINUOUS_ONLY,
	TRIGGER_ON_INTERACTION_AND_CONTINUOUS,
};
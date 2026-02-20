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

UENUM(BlueprintType)
enum class EHeistEquipmentType : uint8
{
	NONE,
	PISTOL,
	SIZE_CHANGE_TOOL,
};


const float TINY_SIZE_MULT = 0.001f;
const float MEDIUM_SIZE_MULT = 1000.0f;
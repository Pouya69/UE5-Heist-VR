

#include "AnimNotify_PlayDefaultCourierSound.h"

#include "AI/CourierCharacter.h"
#include "Components/AudioComponent.h"

UAnimNotify_PlayDefaultCourierSound::UAnimNotify_PlayDefaultCourierSound()
{
	bIsDefaultCutsceneSound = true;
	
	bShouldFireInEditor = false;
	
	VolumeMultiplier = 1.0f;
}

void UAnimNotify_PlayDefaultCourierSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	const ACourierCharacter* CourierRef = Cast<ACourierCharacter>(MeshComp->GetOwner());
	if (!CourierRef || (bIsDefaultCutsceneSound && CourierRef->bHasFinishedFirstCutscene)) return;
	
	
	CourierRef->CourierDefaultSoundsAudioComponent->Stop();
	CourierRef->CourierDefaultSoundsAudioComponent->SetSound(SoundToPlay);
	CourierRef->CourierDefaultSoundsAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
	CourierRef->CourierDefaultSoundsAudioComponent->Play();
}

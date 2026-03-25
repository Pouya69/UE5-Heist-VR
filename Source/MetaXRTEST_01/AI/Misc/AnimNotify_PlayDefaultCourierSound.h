// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlayDefaultCourierSound.generated.h"

/**
 * 
 */
UCLASS()
class METAXRTEST_01_API UAnimNotify_PlayDefaultCourierSound : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAnimNotify_PlayDefaultCourierSound();
	
protected:
	// If true, if bdefaultCutscene is false in Courier Character, it will not play.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sounds")
		bool bIsDefaultCutsceneSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sounds")
		TObjectPtr<USoundBase> SoundToPlay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sounds")
		float VolumeMultiplier;
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

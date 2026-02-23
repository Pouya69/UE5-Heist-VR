// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/HeistInteractionInterface.h"
#include "GameFramework/Actor.h"
#include "Grabbable.generated.h"


enum class EGrabTypeBase : uint8;
class UHeistMotionControllerComponent;
class UHeistGrabComponent;

UCLASS()
class METAXRTEST_01_API AGrabbable : public AActor, public IHeistInteractionInterface
{
	GENERATED_BODY()

public:
	virtual bool IsRemoteGrabbable_Implementation() const override;
	
	// Explicit control
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Remote Grab")
		bool IsRemoteGrabbable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size")
		bool bCanChangeSize;
	
	// This needs to be true if you don't want to scale this by itself.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size")
		bool bIsConnectedToAnotherActor;
	
	AGrabbable();
	
	FTimerHandle RemoteGrabTimerHandle;
		
	UPROPERTY(BlueprintReadOnly, Category = "Size")
		bool bIsGrabbableActive;
	
	UFUNCTION(BlueprintCallable, Category = "Components")
		virtual UPrimitiveComponent* GetMainPrimitiveComponent() const;
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size")
		EHeistSize CurrentSize;
	
	UFUNCTION(BlueprintCallable, Category = "Grab")
		virtual EGrabTypeBase GetGrabType() const;
	
	FVector StartingScale;
	
	UFUNCTION(BlueprintCallable, Category = "Grab")
		virtual void ForceRelease();
	
	UFUNCTION(BlueprintCallable, Category = "Size")
		void ToggleActivateGrabbable(const bool bActive);	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UStaticMeshComponent> BaseMeshComponent;
	
protected:
	UFUNCTION()
		virtual void OnPlayerChangeSize(EHeistSize NewPlayerSize);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UHeistGrabComponent> GrabComponent;
	
	virtual void SetIsInFocus_Implementation(const bool bIsInFocus) override;
	
	virtual bool RemoteGrab_Implementation() override;
	
	virtual bool IsGrabbable_Implementation(const FName BoneHit) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials")
		TObjectPtr<UMaterialInterface> InFocusMaterial;
	
	virtual bool GetGrabComponents_Implementation(TArray<UHeistGrabComponent*>& OutGrabComponents) override;
	
	virtual EHeistGrabHandState GetHandAnimationType_Implementation() const override;
	
	
	
	virtual EHeistSize GetCurrentSizeOfGameObject_Implementation() override;
	virtual void SetNewSizeTo_Implementation(EHeistSize NewSize) override;
	
	UFUNCTION()
		void OnReleased_Default(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	virtual void PostInitializeComponents() override;
};

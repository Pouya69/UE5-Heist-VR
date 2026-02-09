// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Grabbable.h"
#include "SizeChangeMachine.generated.h"

enum class EHeistSize : uint8;
class UHeistMotionControllerComponent;

UCLASS()
class METAXRTEST_01_API ASizeChangeMachine : public AGrabbable
{
	GENERATED_BODY()

public:
	ASizeChangeMachine();
	
	UFUNCTION(BlueprintCallable, Category = "Size Change Machine")
		bool IsReadyToUseSizeMachine() const;
	
	UFUNCTION(BlueprintCallable, Category = "Size Change Machine")
		bool IsReadyToPressButtons() const;
	
	UFUNCTION(BlueprintCallable, Category = "Size Change Machine")
		void PressedButtonForSizeChange(EControllerHand WhichHand, const bool bIsPressed);
	
	UFUNCTION(BlueprintCallable, Category = "Size Change Machine")
		bool ChangePlayerSize(const EHeistSize NewPlayerSize);
	
	UPROPERTY(BlueprintReadOnly, Category = "Size Change Machine")
		bool bIsLeftOnePressed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Size Change Machine")
		bool bIsRightOnePressed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Size Change Machine")
		float TimeGrabbedAndPressedBoth;
	
	UPROPERTY(EditAnywhere, Category = "Size Change Machine | Grab Config")
		FVector GrabOffset;
	
	UPROPERTY(EditAnywhere, Category = "Size Change Machine | Grab Config")
		FVector GrabOffsetRotation;

protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UStaticMeshComponent> FirstHandleMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UHeistGrabComponent> SecondGrabComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UStaticMeshComponent> SecondHandleMeshComponent;
	
	UFUNCTION()
		void OnHandleGrabbed(UHeistGrabComponent* GrabbedComponent, UHeistMotionControllerComponent* MotionControllerRef);
	
	UFUNCTION()
		void OnHandleReleased(UHeistGrabComponent* ReleasedComponent, UHeistMotionControllerComponent* MotionControllerRef);

public:
	virtual void Tick(float DeltaTime) override;
};

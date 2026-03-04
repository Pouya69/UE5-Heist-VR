// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/HeistInteractionInterface.h"
#include "GameFramework/Actor.h"
#include "HeistButton.generated.h"

class USphereComponent;
class UPhysicsConstraintComponent;

UCLASS()
class METAXRTEST_01_API AHeistButton : public AActor, public IHeistInteractionInterface
{
	GENERATED_BODY()

public:
	AHeistButton();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> ButtonBaseMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> ButtonMovingMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<USphereComponent> TriggerSphereOverlapComponent;
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Size")
		EHeistSize CurrentSize;
	
	UFUNCTION()
		void OnPlayerChangeSize(EHeistSize NewPlayerSize);
	
	virtual void Interact_Implementation() override;
	
	UFUNCTION()
		void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
		void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	virtual void PostInitializeComponents() override;
	
	float StartingButtonPosition;
	
	UPROPERTY(BlueprintReadOnly, Category="Button")
		bool bIsButtonActive;
	
	// Difference from Starting Position.
	UPROPERTY(EditAnywhere, Category="Button")
		float ButtonPushedInDifference;
	
	// If <= 0, We use anything. If not, the pushed object needs to be more than this mass.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Button")
		float MinimumMass;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Button")
		bool bIsPowered;
		
	
	UPROPERTY(EditAnywhere, Category="Button")
		float ButtonActivationForceThreshold;

	UPROPERTY(EditAnywhere, Category="Button")
		float ButtonReturnToStartSpeed;
	
	UPROPERTY(EditAnywhere, Category="Button")
		float ButtonPushInSpeed;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Button")
		TObjectPtr<AActor> LinkedActor;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Button")
		void ButtonAtStartingPoint();
	
	
	

public:
	virtual void Tick(float DeltaTime) override;
};

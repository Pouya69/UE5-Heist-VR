// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/HeistInteractionInterface.h"
#include "GameFramework/Actor.h"
#include "HeistMagnetCrane.generated.h"

class UBoxComponent;
class URadialForceComponent;

UCLASS(Abstract)
class METAXRTEST_01_API AHeistMagnetCrane : public AActor, public IHeistInteractionInterface
{
	GENERATED_BODY()

public:
	AHeistMagnetCrane();

protected:
	virtual void BeginPlay() override;
	
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<USkeletalMeshComponent> CraneSkeletalMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<URadialForceComponent> RadialForceComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		TObjectPtr<UBoxComponent> OverlapDetectionBoxComp;
	
	UFUNCTION()
		void OnObjectEnteredOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UFUNCTION()
		void OnObjectExitOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY()
		TArray<UPrimitiveComponent*> CompsAttached;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
		FName RadialForceComponentSocketAttachment;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "IK")
		FVector IK_Location;
	
	// RELATIVE TO BASE
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK | Config")
		float MinRotationYaw;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK | Config")
		float StartingDistance;
	
	// RELATIVE TO BASE
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK | Config")
		float MaxRotationYaw;
	
	// RELATIVE TO BASE
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK | Config")
	float MinHeight;
	
	// RELATIVE TO BASE
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK | Config")
		float MaxHeight;
	
	UPROPERTY(BlueprintReadOnly, Category = "IK")
		float CurrentYawNormalized;
	
	UPROPERTY(BlueprintReadOnly, Category = "IK")
		float CurrentHeightNormalized;
	
	UFUNCTION()
		void OnPlayerChangeSize(EHeistSize NewPlayerSize);
	
	virtual void Interact_Implementation() override;
	virtual void Interact_Reset_Implementation() override;
	virtual void SetAmount_Implementation(const float NewAmount) override;
	virtual void SetAmount_02_Implementation(const float NewAmount) override;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeistPlayerMainComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class METAXRTEST_01_API UHeistPlayerMainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeistPlayerMainComponent();

protected:
	
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		float RemoteGrabRange;
	
	// Sphere Sweep Radius
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		float RemoteGrabRadiusCheck;
	
	UPROPERTY()
		TObjectPtr<USkeletalMeshComponent> RightGhostHandRef;
	UPROPERTY()
		TObjectPtr<USkeletalMeshComponent> LeftGhostHandRef;
	
	UPROPERTY()
		TObjectPtr<USkeletalMeshComponent> RightPhysicsHandRef;
	UPROPERTY()
		TObjectPtr<USkeletalMeshComponent> LeftPhysicsHandRef;
	
	// Between the hand velocity (now - previous) AND normal of surface.
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		float MinDotProductRemoteGrabThreshold;
	
	// The Vector Length of hand movement direction has to be >= this for it to be gravitated.
	// E.g. 3.0
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		float MinForceThresholdVectorLength;
	
	// This will be added as a default for a booster. (to go against gravity etc.)
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		FVector RemoteGrabForceAddition;

	// For mapping distance to force.
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		FVector2D RemoteGrabDistanceRange;
	
	// For mapping distance to force.
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		FVector2D RemoteGrabForceRange;

public:
	UFUNCTION(BlueprintCallable)
	void InitializePlayerComponent(USkeletalMeshComponent* InRightGhostHandRef, USkeletalMeshComponent* InRightPhysicsHandRef,
		USkeletalMeshComponent* InLeftGhostHandRef, USkeletalMeshComponent* InLeftPhysicsHandRef);
	
	UFUNCTION(BlueprintCallable, Category="Remote Grab")
		void RemoteGrabRight();
	
	UFUNCTION(BlueprintCallable, Category="Remote Grab")
		void RemoteGrabLeft();
	
	UPROPERTY(BlueprintReadOnly, Category="Remote Grab")
		UPrimitiveComponent* CurrentGrabInFocus_R;
	
	UPROPERTY(BlueprintReadOnly, Category="Remote Grab")
		UPrimitiveComponent* CurrentGrabInFocus_L;
		
	UPROPERTY(BlueprintReadOnly, Category="Hands")
		FVector RightHandPreviousLocation;
	
	UPROPERTY(BlueprintReadOnly, Category="Hands")
		FVector LeftHandPreviousLocation;
};


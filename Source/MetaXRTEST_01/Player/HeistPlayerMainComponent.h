// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeistPlayerMainComponent.generated.h"


enum class EHeistEquipmentType : uint8;
class UPhysicsConstraintComponent;
enum class EHeistSize : uint8;
class AHeistPistol;
class UCameraComponent;
class UHeistMotionControllerComponent;

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
	
	UPROPERTY()
		TObjectPtr<UHeistMotionControllerComponent> RightMotionControllerRef;
	UPROPERTY()
		TObjectPtr<UHeistMotionControllerComponent> LeftMotionControllerRef;
	
	UPROPERTY()
		TObjectPtr<UCameraComponent> CameraComponentRef;
	
	// Between the hand velocity (now - previous) AND normal of surface.
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		float MinDotProductRemoteGrabThreshold;
	
	// Between the hand velocity (now - previous) AND the camera looking direction.
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		float MinDotProductCameraRemoteGrabThreshold;
	
	// The Vector Length of hand movement direction has to be >= this for it to be gravitated.
	// E.g. 3.0
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		float MinForceThresholdVectorLength;
	
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		float LengthThresholdForAllowingRemoteGrab;
	
	// This will be added as a default for a booster. (to go against gravity etc.)
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		FVector RemoteGrabForceAddition;

	// For mapping distance to force.
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		FVector2D RemoteGrabDistanceRange;
	
	// For mapping distance to force.
	UPROPERTY(EditAnywhere, Category = "Remote Grab")
		FVector2D RemoteGrabForceRange;
	
	UPROPERTY(BlueprintReadOnly, Category="Hand Equipments")
		TObjectPtr<AHeistPistol> PistolAttachedToHand;
	
	float PlayerRadius;
	float PlayerCapsuleHalfHeight;

public:
	UPROPERTY(BlueprintReadWrite, Category="Equipment")
		EHeistEquipmentType CurrentEquippedEquipment;	
	
	UFUNCTION(BlueprintCallable, Category="Procedural Grab")
		void StartLockHands(const bool bIsRightHand, const FVector& HandLockLocation);
	
	UFUNCTION(BlueprintCallable, Category="Procedural Grab")
		void StopLockHands(const bool bIsRightHand);
	
	void LockHands_Tick(const float& DeltaTime, const bool bIsRightHand);
	
	UPROPERTY(BlueprintReadOnly, Category="Procedural Grab")
		FVector LeftGrabHandLockLocation;
	
	// For interpolating TO the controller from the locked location.
	UPROPERTY(EditDefaultsOnly, Category="Procedural Grab")
		float GrabInterpToRealHandsSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category="Procedural Grab")
		FVector RightGrabHandLockLocation;
	
	UPROPERTY(BlueprintReadWrite, Category="Size")
		EHeistSize CurrentSize;
	
	UPROPERTY()
		UPhysicsConstraintComponent* RightHandPhysicsConstraint;
	
	UPROPERTY()
		UPhysicsConstraintComponent* LeftHandPhysicsConstraint;
	
	UPROPERTY()
		UPhysicsConstraintComponent* RightHandGrabPhysicsConstraint;
	
	UPROPERTY()
		UPhysicsConstraintComponent* LeftHandGrabPhysicsConstraint;
	
	UFUNCTION(BlueprintCallable)
	void InitializePlayerComponent(EHeistSize InCurrentSize, const float InPlayerRadius, const float InPlayerCapsuleHalfHeight, USkeletalMeshComponent* InRightGhostHandRef, USkeletalMeshComponent* InRightPhysicsHandRef,
		USkeletalMeshComponent* InLeftGhostHandRef, USkeletalMeshComponent* InLeftPhysicsHandRef, UPhysicsConstraintComponent* InRightHandPhysicsConstraint, UPhysicsConstraintComponent* InLeftHandPhysicsConstraint,
		UHeistMotionControllerComponent* InLeftMotionControllerRef, UHeistMotionControllerComponent* InRightMotionControllerRef,
		UPhysicsConstraintComponent* InRightHandGrabPhysicsConstraint, UPhysicsConstraintComponent* InLeftHandGrabPhysicsConstraint, UCameraComponent* InCameraComponent, AHeistPistol* InHeistPistol);
	
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
	
	UFUNCTION(BlueprintCallable, Category="Hands")
		void CustomGrab_Tick(const float& DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category="Hands | Pistol")
		void Custom_PistolTick(const float& Alpha);
	
	UFUNCTION(BlueprintCallable, Category="Hands | Pistol")
		void TogglePistolEnabled(const bool bEnabled);
	
	bool bCanRemoteGrab_R;
	bool bCanRemoteGrab_L;
	
	UFUNCTION(BlueprintCallable, Category="Change Size")
		bool ChangeSizeTo(EHeistSize NewSize, const FVector NewLocation);
	
	UFUNCTION()
		void PlayerChangedSize();
	
	// One-time used only
	UFUNCTION(BlueprintCallable, Category="Hands | Pistol")
		void PickedUpPistol();
};


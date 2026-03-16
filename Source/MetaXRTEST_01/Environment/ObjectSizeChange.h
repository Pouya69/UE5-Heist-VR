
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectSizeChange.generated.h"

class AGrabbable;
class USphereComponent;
enum class EHeistSize : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangedSizeOfObject, AGrabbable*, GrabbableChanged);

UCLASS()
class METAXRTEST_01_API AObjectSizeChange : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Size Change")
		FOnChangedSizeOfObject OnChangedSizeOfObject;
	
	AObjectSizeChange();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Size Change")
		EHeistSize CurrentSize;
	
	UFUNCTION(BlueprintCallable, Category="Size Change")
		void SpitOutObject(AGrabbable* GrabbableToSpitOut);
	
	UPROPERTY(BlueprintReadOnly, Category="Size Change")
		FTransform SpitOutTransform;
	
	UFUNCTION()
		void OnPlayerChangeSize(EHeistSize NewPlayerSize);
	
	// For cutscenes etc.
	UFUNCTION(BLueprintCallable, Category="Size Change")
		void ForceChangeSizeOfObject(AGrabbable* Grabbable);
	
protected:
	UPROPERTY(EditInstanceOnly, Category="Size Change")
		float MinForceOnSpitOut;
	
	TArray<AGrabbable*> GrabbablesSpitting;
	
	UPROPERTY(EditInstanceOnly, Category="Size Change")
		float MaxForceOnSpitOut;
	
	virtual void PostInitializeComponents() override;
	
	UFUNCTION()
		void SpitOutGrabbableAfterRelease(AGrabbable* GrabbableToSpitOut);
	
	UPROPERTY(EditAnywhere, Category="Size Change")
		float AdditionToSpitTransform;
	
	UFUNCTION()
		void OnObjectEnteredVaccum(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);	
	
	UPROPERTY()
		UPrimitiveComponent* RecentPrimitiveComponent;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Size Change")
		TObjectPtr<AObjectSizeChange> OtherObjectSizeChangerSide;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> BaseMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		USceneComponent* SpitOutDirectionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<USphereComponent> ObjectOverlapSphereComponent;
	
};

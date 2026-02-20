
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectSizeChange.generated.h"

class AGrabbable;
class USphereComponent;
enum class EHeistSize : uint8;

UCLASS()
class METAXRTEST_01_API AObjectSizeChange : public AActor
{
	GENERATED_BODY()

public:
	AObjectSizeChange();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Size Change")
		EHeistSize CurrentSize;
	
	UFUNCTION(BlueprintCallable, Category="Size Change")
		void SpitOutObject(AGrabbable* GrabbableToSpitOut);
	
	UPROPERTY(BlueprintReadOnly, Category="Size Change")
		FTransform SpitOutTransform;
	
	UFUNCTION()
		void OnPlayerChangeSize(EHeistSize NewPlayerSize);
	
protected:
	virtual void PostInitializeComponents() override;
	
	UFUNCTION()
		void OnObjectEnteredVaccum(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);	
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Size Change")
		TObjectPtr<AObjectSizeChange> OtherObjectSizeChangerSide;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<UStaticMeshComponent> BaseMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		USceneComponent* SpitOutDirectionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
		TObjectPtr<USphereComponent> ObjectOverlapSphereComponent;
	
};

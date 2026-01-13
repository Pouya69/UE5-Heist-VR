// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Puzzle.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPuzzleCompleted, APuzzle*, PuzzleSolved, APlayerController*, PlayerController);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPuzzleObjectiveFinished, APuzzle*, Puzzle, const uint8, FinishedPuzzleObjectiveIndex, APlayerController*, PlayerController);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPuzzleFailed, APuzzle*, PuzzleFailed, const uint8, FailedPuzzleObjectiveIndex, APlayerController*, PlayerController);


UCLASS()
class METAXRTEST_01_API APuzzle : public AActor
{
	GENERATED_BODY()

public:
	APuzzle();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
		FOnPuzzleCompleted OnPuzzleCompleted;
	
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
		FOnPuzzleObjectiveFinished OnPuzzleObjectiveFinished;
	
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
		FOnPuzzleFailed  OnPuzzleFailed;
};

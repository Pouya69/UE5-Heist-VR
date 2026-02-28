// Fill out your copyright notice in the Description page of Project Settings.


#include "CourierController.h"


ACourierController::ACourierController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACourierController::BeginPlay()
{
	Super::BeginPlay();
}

void ACourierController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	RunBehaviorTree(CourierBehaviourDefault);
}

void ACourierController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


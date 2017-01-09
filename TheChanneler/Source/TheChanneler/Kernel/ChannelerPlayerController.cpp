// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerPlayerController.h"
#include "../Input/InputDeviceManager.h"

AChannelerPlayerController::AChannelerPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	InputDeviceManager = NewObject<UInputDeviceManager>(this, UInputDeviceManager::StaticClass(), TEXT("InputDeviceManager"));
}

void AChannelerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(true);
	EnableInput(this);
}

void AChannelerPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(InputDeviceManager != nullptr)
		InputDeviceManager->Tick();
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerPlayerController.h"
#include "../Input/InputDeviceManager.h"
#include "../Utils/ChannelerUtils.h"

AChannelerPlayerController::AChannelerPlayerController()
	: InputDeviceManager(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	InputDeviceManager = NewObject<UInputDeviceManager>(this, UInputDeviceManager::StaticClass(), TEXT("InputDeviceManager"));
	UChannelerUtils::SetInputDeviceManager(InputDeviceManager);
}

void AChannelerPlayerController::BeginPlay()
{
	if (InputDeviceManager == nullptr)
	{
		InputDeviceManager = NewObject<UInputDeviceManager>(this, UInputDeviceManager::StaticClass(), TEXT("InputDeviceManager"));
		UChannelerUtils::SetInputDeviceManager(InputDeviceManager);
	}
	
	Super::BeginPlay();

	SetActorTickEnabled(true);
	EnableInput(this);
}

void AChannelerPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	InputDeviceManager->Tick();
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "InputDeviceManager.generated.h"

/**
 * Enumeration for determining the device which input was last known to come from.
 */
UENUM(BlueprintType)
enum class EInputDevices : uint8
{
	ID_KBM			UMETA(DisplayName = "Keyboard/Mouse"),
	ID_Gamepad		UMETA(DisplayName = "Gamepad")
};

/**
 * 
 */
UCLASS(BlueprintType)
class THECHANNELER_API UInputDeviceManager : public UObject
{
	GENERATED_BODY()
	
public:
	UInputDeviceManager();
	void Tick();
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	EInputDevices GetLastKnownInputDevice() const;

	UFUNCTION(BlueprintCallable, Category = "Input")
	EInputDevices GetPreviousFramesLastKnownInputDevice() const;

	class APlayerController* Controller;

private:
	bool WasKMBInputJustDetected() const;
	bool WasGamepadInputJustDetected() const;

	EInputDevices mLastKnownInputDevice;
	EInputDevices mPreviousFramesLastKnownInputDevice;

	TArray<FKey> mKBMKeys;
	TArray<FKey> mControllerKeys;
};

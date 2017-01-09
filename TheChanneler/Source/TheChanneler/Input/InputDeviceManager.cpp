// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "InputDeviceManager.h"

UInputDeviceManager::UInputDeviceManager() :
	mLastKnownInputDevice(EInputDevices::ID_KBM), mPreviousFramesLastKnownInputDevice(mLastKnownInputDevice)
{
	mKBMKeys.Add(EKeys::MouseX);
	mKBMKeys.Add(EKeys::MouseY);
	mKBMKeys.Add(EKeys::LeftMouseButton);
	mKBMKeys.Add(EKeys::RightMouseButton);
	mKBMKeys.Add(EKeys::W);
	mKBMKeys.Add(EKeys::A);
	mKBMKeys.Add(EKeys::A);
	mKBMKeys.Add(EKeys::D);
	mKBMKeys.Add(EKeys::P);
	mKBMKeys.Add(EKeys::Tab);
	mKBMKeys.Add(EKeys::Enter);
	mKBMKeys.Add(EKeys::Escape);
	mKBMKeys.Add(EKeys::SpaceBar);

	mControllerKeys.Add(EKeys::Gamepad_LeftX);
	mControllerKeys.Add(EKeys::Gamepad_LeftY);
	mControllerKeys.Add(EKeys::Gamepad_RightX);
	mControllerKeys.Add(EKeys::Gamepad_RightY);
	mControllerKeys.Add(EKeys::Gamepad_LeftTriggerAxis);
	mControllerKeys.Add(EKeys::Gamepad_RightTriggerAxis);
	mControllerKeys.Add(EKeys::Gamepad_LeftThumbstick);
	mControllerKeys.Add(EKeys::Gamepad_RightThumbstick);
	mControllerKeys.Add(EKeys::Gamepad_Special_Left);
	mControllerKeys.Add(EKeys::Gamepad_Special_Right);
	mControllerKeys.Add(EKeys::Gamepad_FaceButton_Bottom);
	mControllerKeys.Add(EKeys::Gamepad_FaceButton_Right);
	mControllerKeys.Add(EKeys::Gamepad_FaceButton_Left);
	mControllerKeys.Add(EKeys::Gamepad_FaceButton_Top);
	mControllerKeys.Add(EKeys::Gamepad_LeftShoulder);
	mControllerKeys.Add(EKeys::Gamepad_RightShoulder);
	mControllerKeys.Add(EKeys::Gamepad_LeftTrigger);
	mControllerKeys.Add(EKeys::Gamepad_RightTrigger);
	mControllerKeys.Add(EKeys::Gamepad_DPad_Up);
	mControllerKeys.Add(EKeys::Gamepad_DPad_Down);
	mControllerKeys.Add(EKeys::Gamepad_DPad_Right);
	mControllerKeys.Add(EKeys::Gamepad_DPad_Left);
	mControllerKeys.Add(EKeys::Gamepad_LeftStick_Up);
	mControllerKeys.Add(EKeys::Gamepad_LeftStick_Down);
	mControllerKeys.Add(EKeys::Gamepad_LeftStick_Right);
	mControllerKeys.Add(EKeys::Gamepad_LeftStick_Left);
	mControllerKeys.Add(EKeys::Gamepad_RightStick_Up);
	mControllerKeys.Add(EKeys::Gamepad_RightStick_Down);
	mControllerKeys.Add(EKeys::Gamepad_RightStick_Right);
	mControllerKeys.Add(EKeys::Gamepad_RightStick_Left);
}

void UInputDeviceManager::Tick()
{
	mPreviousFramesLastKnownInputDevice = mLastKnownInputDevice;

	if (WasGamepadInputJustDetected())
	{
		mLastKnownInputDevice = EInputDevices::ID_Gamepad;
	}
	else if (WasKMBInputJustDetected())
	{
		mLastKnownInputDevice = EInputDevices::ID_KBM;
	}
}

EInputDevices UInputDeviceManager::GetLastKnownInputDevice() const
{
	return mLastKnownInputDevice;
}

EInputDevices UInputDeviceManager::GetPreviousFramesLastKnownInputDevice() const
{
	return mPreviousFramesLastKnownInputDevice;
}

bool UInputDeviceManager::WasKMBInputJustDetected() const
{
	if (GEngine == nullptr || GEngine->GetWorld() == nullptr || GEngine->GetWorld()->GetFirstPlayerController() == nullptr)
		return false;

	APlayerController* controller = GEngine->GetWorld()->GetFirstPlayerController();
	if (controller != nullptr)
	{
		for (FKey key : mKBMKeys)
		{
			if (controller->WasInputKeyJustPressed(key))
				return true;
		}
	}
	return false;
}

bool UInputDeviceManager::WasGamepadInputJustDetected() const
{
	if (GEngine == nullptr || GEngine->GetWorld() == nullptr || GEngine->GetWorld()->GetFirstPlayerController() == nullptr)
		return false;

	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (controller != nullptr)
	{
		for (FKey key : mControllerKeys)
		{
			if (controller->WasInputKeyJustPressed(key))
				return true;
		}
	}
	return false;
}
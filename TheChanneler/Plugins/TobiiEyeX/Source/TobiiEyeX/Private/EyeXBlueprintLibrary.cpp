// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXBlueprintLibrary.h"

IEyeXPlugin* UEyeXBlueprintLibrary::EyeX;

UEyeXBlueprintLibrary::UEyeXBlueprintLibrary(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void UEyeXBlueprintLibrary::GetGazePoint(EEyeXGazePointDataMode::Type Mode, FVector2D& GazePoint, float& TimeStamp, bool& bHasValue)
{
	FEyeXGazePoint data = IsEyeXModuleAvailable() ? EyeX->GetGazePoint(Mode) : FEyeXGazePoint::Invalid();
	GazePoint = data.Value;
	TimeStamp = data.TimeStamp;
	bHasValue = data.bHasValue;
}

void UEyeXBlueprintLibrary::GetLastFixation(EEyeXFixationDataMode::Type Mode, FVector2D& GazePoint, float& BeginTimeStamp, float& Duration, bool& bHasValue)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->GetLastFixation(Mode) : FEyeXFixationDataPoint::Invalid();
	GazePoint = data.GazePoint;
	BeginTimeStamp = data.BeginTimeStamp;
	Duration = data.Duration;
	bHasValue = data.bHasValue;
}

void UEyeXBlueprintLibrary::GetOngoingFixation(EEyeXFixationDataMode::Type Mode, FVector2D& GazePoint, float& BeginTimeStamp, float& Duration, bool& bHasValue)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->GetOngoingFixation(Mode) : FEyeXFixationDataPoint::Invalid();
	GazePoint = data.GazePoint;
	BeginTimeStamp = data.BeginTimeStamp;
	Duration = data.Duration;
	bHasValue = data.bHasValue;
}

void UEyeXBlueprintLibrary::Get3DGazePoint(EEyeXGazePointDataMode::Type Mode, FVector& GazePoint, bool& bHasValue)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->Get3DGazePoint(Mode) : TEyeXMaybeValue<FVector>(FVector::ZeroVector, false);
	GazePoint = data.Value;
	bHasValue = data.bHasValue;
}

void UEyeXBlueprintLibrary::GetEyePosition(FVector& LeftEye, FVector& RightEye, bool& bIsLeftEyeValid, bool& bIsRightEyeValid, float& TimeStamp)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->GetEyePosition() : FEyeXEyePosition::Invalid();
	LeftEye = data.LeftEye;
	RightEye = data.RightEye;
	bIsLeftEyeValid = data.bIsLeftEyeValid;
	bIsRightEyeValid = data.bIsRightEyeValid;
	TimeStamp = data.TimeStamp;
}

void UEyeXBlueprintLibrary::GetHeadRotation(FRotator& Rotation, bool& bHasValue)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->GetHeadRotation() : TEyeXMaybeValue<FRotator>(FRotator::ZeroRotator, false);
	Rotation = data.Value;
	bHasValue = data.bHasValue;
}

void UEyeXBlueprintLibrary::GetScreenBounds(int32& X, int32&Y, int32& Width, int32& Height, bool& bHasValue)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->GetScreenBounds() : TEyeXMaybeValue<FEyeXScreenBounds>(FEyeXScreenBounds(), false);
	X = data.Value.X;
	Y = data.Value.Y;
	Width = data.Value.Width;
	Height = data.Value.Height;
	bHasValue = data.bHasValue;
}

void UEyeXBlueprintLibrary::GetDisplaySize(FVector2D& DisplaySize, bool& bHasValue)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->GetDisplaySize() : TEyeXMaybeValue<FVector2D>(FVector2D::ZeroVector, false);
	DisplaySize = data.Value;
	bHasValue = data.bHasValue;
}

void UEyeXBlueprintLibrary::GetEyeTrackingDeviceStatus(TEnumAsByte<EEyeXDeviceStatus::Type>& DeviceStatus)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->GetEyeTrackingDeviceStatus() : EEyeXDeviceStatus::Unknown;
	DeviceStatus = data;
}

void UEyeXBlueprintLibrary::GetUserPresence(TEnumAsByte<EEyeXUserPresence::Type>& UserPresence)
{
	auto data = IsEyeXModuleAvailable() ? EyeX->GetUserPresence() : EEyeXUserPresence::Unknown;
	UserPresence = data;
}

TEnumAsByte<EEyeXEmulationMode::Type> UEyeXBlueprintLibrary::GetEmulationMode()
{
	if (IsEyeXModuleAvailable())
	{
		return EyeX->GetEmulationMode();
	}

	return EEyeXEmulationMode::Disabled;
}

void UEyeXBlueprintLibrary::SetEmulationMode(TEnumAsByte<EEyeXEmulationMode::Type> Mode)
{
	if (IsEyeXModuleAvailable())
	{
		EyeX->SetEmulationMode(Mode);
	}
}

bool UEyeXBlueprintLibrary::IsEyeXModuleAvailable()
{
	static bool bIsModuleInitialized = false;

	if (!bIsModuleInitialized)
	{
		bIsModuleInitialized = true;

		if (IEyeXPlugin::IsAvailable())
		{
			EyeX = &IEyeXPlugin::Get();
			return true;
		}
		else
		{
			UE_LOG(LogEyeX, Error, TEXT("TobiiEyeX Module is unavailable."));
			EyeX = nullptr;
			return false;
		}
	}

	return EyeX != nullptr;
}

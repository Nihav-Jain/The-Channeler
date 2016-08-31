// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "EyeXTypes.h"
#include "EyeXBlueprintLibrary.generated.h"

class IEyeXPlugin;

/**
 * A blueprint library class to expose the functionality of the EyeX plugin to blueprints
 * in all blueprint contexts.
 */
UCLASS()
class UEyeXBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
public:

	/**
	 * Gets the latest eye-gaze data point of the specified mode.
	 *
	 * @param Mode      - Specifies the kind of data processing to be applied by the EyeX Engine.
	 *                    Unfiltered: Unfiltered data from the eye tracker. The most responsive mode, 
	 *                    but can jitter a lot.
	 *                    LightlyFiltered: A light filtering applied by the EyeX Engine. Less 
	 *                    jittering but also slightly less responsive.
	 * @param GazePoint - The gaze point in viewport coordinates (pixels).
	 * @param TimeStamp - The time this data was created in milliseconds.
	 * @param bHasValue - Indicates whether the data point is valid.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void GetGazePoint(EEyeXGazePointDataMode::Type Mode, FVector2D& GazePoint, float& Timestamp, bool& bHasValue);

	/** 
	 * Gets the latest eye-gaze data point of the specified mode in 3D space.
	 * The origin of this coordinate system is in the center of the screen with the X-axis pointing
	 * into the screen (in the direction away from the user), the Y-axis to the right and the
	 * Z-axis pointing upwards.
	 * The data is in cm.
	 *
	 * @param Mode      - Specifies the kind of data processing to be applied by the EyeX Engine.
	 * @param GazePoint - the gaze point in 3D coordinates.
	 * @param bHasValue - Indicates whether the data point is valid.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void Get3DGazePoint(EEyeXGazePointDataMode::Type Mode, FVector& GazePoint, bool& bHasValue);

	/**
	 * Gets the last complete fixation. 
	 * As soon as an ongoing fixation is complete it will be saved as the last fixation. While a new
	 * fixation is ongoing, the last fixation will not be changed.
	 * The unique BeginTimeStamp property can be used to compare if fixations are different from 
	 * one another.
	 *
	 * @param Mode           - Specifies the kind of data processing to be applied by the EyeX Engine.
	 * @param GazePoint      - The gaze point in viewport coordinates (pixels).
	 * @param BeginTimeStamp - The time the fixation started in milliseconds.
	 * @param Duration       - The total duration of this fixation in milliseconds.
	 * @param bHasValue      - Indicates whether the data point is valid.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void GetLastFixation(EEyeXFixationDataMode::Type Mode, FVector2D& GazePoint, float& BeginTimeStamp, float& Duration, bool& bHasValue);

	/**
	 * Gets the latest fixation data for the ongoing fixation.
	 * When a fixation is ongoing, the only changing properties are the gaze point and the duration. 
	 * The gaze point converges towards the most likely point the eyes are lingering at, and the 
	 * duration increases until the fixation reaches the fixation is complete. When the ongoing 
	 * fixation is complete, it is set as the last fixation (retrievable with GetLastFixation), and
	 * the ongoing fixation is set to an invalid value until a new fixation has started.
	 *
	 * @param Mode           - Specifies the kind of data processing to be applied by the EyeX Engine.
	 * @param GazePoint      - The gaze point in viewport coordinates (pixels).
	 * @param BeginTimeStamp - The time the fixation started in milliseconds.
	 * @param Duration       - The duration of this ongoing fixation since it began, in milliseconds.
	 * @param bHasValue      - Indicates whether the data point is valid.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
    static void GetOngoingFixation(EEyeXFixationDataMode::Type Mode, FVector2D& GazePoint, float& BeginTimeStamp, float& Duration, bool& bHasValue);

	/**
	 * Gets the latest eye position data in 3D space.
	 * The origin of this coordinate system is in the center of the screen with the X-axis pointing
	 * into the screen (in the direction away from the user), the Y-axis to the right and the
	 * Z-axis pointing upwards.
	 * The data is in cm.
	 *
	 * @param LeftEye          - The coordinate of the left eye in cm.
	 * @param RightEye         - The coordinate of the right eye in cm.
	 * @param bIsLeftEyeValid  - True if the left eye position contains valid data. Will be false if 
	 *                           the tracker failed to detect the left eye.
	 * @param bIsRightEyeValid - True if the right eye position contains valid data. Will be false 
	 *                           if the tracker failed to detect the right eye.
	 * @param TimeStamp        - The time this data was created in milliseconds.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void GetEyePosition(FVector& LeftEye, FVector& RightEye, bool& bIsLeftEyeValid, bool& bIsRightEyeValid, float& TimeStamp);

	/**
	 * Returns the rotation of the users head. Note that pitch cannot be calculated using the eye
	 * tracker and is always zero. Both eyes need to be tracked in order to calculate head rotation.
	 *
	 * @param Rotation  - The rotation of the user's head.
	 * @param bHasValue - Indicates whether the data point is valid.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void GetHeadRotation(FRotator& Rotation, bool& bHasValue);

	/**
	 * Returns the bounds of the screen where the eye tracker is mounted, in physical pixels.
	 *
	 * @param X         - The X coordinate of the screen in physical pixels. The primary screen will have X = 0 and Y = 0.
	 * @param Y         - The Y coordinate of the screen in physical pixels.
	 * @param Width     - The width of the screen in physical pixels.
	 * @param Height    - The height of the screen in physical pixels.
	 * @param bHasValue - Indicates whether the value is known.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void GetScreenBounds(int32& X, int32&Y, int32& Width, int32& Height, bool& bHasValue);

	/**
	 * Returns the physical size of the display where the eye tracker is mounted, in millimeters.
	 *
	 * @param DisplaySize - The physical size of the display.
	 * @param bHasValue   - Indicates whether the value is known.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void GetDisplaySize(FVector2D& DisplaySize, bool& bHasValue);

	/**
	 * Returns the status of the eye tracking device.
	 *
	 * @param DeviceStatus - The device status.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void GetEyeTrackingDeviceStatus(TEnumAsByte<EEyeXDeviceStatus::Type>& DeviceStatus);

	/**
	 * Returns the user presence (that is, whether the eye tracker can detect the user's eyes).
	 *
	 * @param UserPresence - The user presence status.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static void GetUserPresence(TEnumAsByte<EEyeXUserPresence::Type>& UserPresence);

	/**
	 * Gets the current emulation mode. The default is no emulation.
	 *
	 * @return The emulation mode.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	static TEnumAsByte<EEyeXEmulationMode::Type> GetEmulationMode();

	/**
	 * Sets the emulation mode for the eye-gaze point. The default is no emulation.
	 *
	 * @param Mode - The emulation mode.
	 */
	UFUNCTION(BlueprintCallable, Category = "EyeX")
	static void SetEmulationMode(TEnumAsByte<EEyeXEmulationMode::Type> Mode);

private:

	static bool IsEyeXModuleAvailable();

private:

	static IEyeXPlugin* EyeX;

};

// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "EyeXTypes.h"

/**
 * The public interface of the TobiiEyeX Module.
 */
class IEyeXPlugin : public IModuleInterface
{
public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though. Your module might have been 
	 * unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IEyeXPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked< IEyeXPlugin >("TobiiEyeX");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if 
	 * IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("TobiiEyeX");
	}

	//==============================================================================
	// Public EyeX API								
	//==============================================================================
	
	/**
	 * Gets the latest eye-gaze data point of the specified mode.
	 * The data is not guaranteed to be the same for an entire tick since it is updated on a 
	 * background thread.
	 *
	 * @param Mode - Specifies the kind of data processing to be applied by the EyeX Engine.
	 * @return the gaze point in viewport coordinates (pixels).
	 */
	virtual FEyeXGazePoint GetGazePoint(EEyeXGazePointDataMode::Type Mode = EEyeXGazePointDataMode::LightlyFiltered) = 0;

	/**
	 * Gets the latest eye-gaze data point of the specified mode in 3D space. 
	 * The origin of this coordinate system is in the center of the screen with the X-axis pointing
	 * into the screen (in the direction away from the user), the Y-axis to the right and the 
	 * Z-axis pointing upwards.
	 * The data is in cm.
	 * The data is not guaranteed to be the same for an entire tick.
	 *
	 * @param Mode - Specifies the kind of data processing to be applied by the EyeX Engine.
	 * @return the gaze point in 3D coordinates.
	 */
	virtual TEyeXMaybeValue<FVector> Get3DGazePoint(EEyeXGazePointDataMode::Type Mode) = 0;

	/**
	 * Gets the last complete fixation.
	 * As soon as an ongoing fixation is complete it will be saved as the last fixation. While a new
	 * fixation is ongoing, the last fixation will not be changed.
	 * The unique BeginTimeStamp property can be used to compare if fixations are different from
	 * one another.
	 *
	 * @param Mode - Specifies the kind of data processing to be applied by the EyeX Engine.
	 * @return the latest complete fixation; gaze point in viewport coordinates.
	 */
	virtual FEyeXFixationDataPoint GetLastFixation(EEyeXFixationDataMode::Type Mode) = 0;

	/**
	 * Gets the latest fixation data for the ongoing fixation.
	 * When a fixation is ongoing, the only changing properties are the gaze point and the duration.
	 * The gaze point converges towards the most likely point the eyes are lingering at, and the
	 * duration increases until the fixation reaches the fixation is complete. When the ongoing
	 * fixation is complete, it is set as the last fixation (retrievable with GetLastFixation), and
	 * the ongoing fixation is set to an invalid value until a new fixation has started.
	 *
	 * @param Mode - Specifies the kind of data processing to be applied by the EyeX Engine.
	 * @return the latest fixation data for ongoing fixation; gaze point in viewport coordinates.
	 */
	virtual FEyeXFixationDataPoint GetOngoingFixation(EEyeXFixationDataMode::Type Mode) = 0;

	/**
	 * Gets the latest eye position data in 3D space.
	 * The origin of this coordinate system is in the center of the screen with the X-axis pointing
	 * into the screen (in the direction away from the user), the Y-axis to the right and the
	 * Z-axis pointing upwards.
	 * The data is in cm.
	 * The data is not guaranteed to be the same for an entire tick.
	 *
	 * @return the eye positions.
	 */
	virtual FEyeXEyePosition GetEyePosition() = 0;

	/**
	 * Gets the rotation of the users head. Note that pitch cannot be calculated using the eye 
	 * tracker and is always zero. Both eyes need to be tracked in order to calculate head rotation.
	 * The data is not guaranteed to be the same for an entire tick.
	 *
	 * @return the rotation of the user's head.
	 */
	virtual TEyeXMaybeValue<FRotator> GetHeadRotation() = 0;

	/**
	 * Gets the bounds of the screen where the eye tracker is mounted, in physical pixels.
	 *
	 * @return the screen bounds.
	 */
	virtual TEyeXMaybeValue<FEyeXScreenBounds> GetScreenBounds() = 0;

	/**
	 * Gets the physical size of the display where the eye tracker is mounted, in millimeters.
	 *
	 * @return the physical size of the display.
	 */
	virtual TEyeXMaybeValue<FVector2D> GetDisplaySize() = 0;

	/**
	 * Gets the status of the eye tracking device.
	 *
	 * @return the device status.
	 */
	virtual EEyeXDeviceStatus::Type GetEyeTrackingDeviceStatus() = 0;

	/** 
	 * Gets a value indicating the presence or non-presence of a user (that is, whether the eye 
	 * tracker can detect the user's eyes or not).
	 *
	 * @return the user presence status.
	 */
	virtual EEyeXUserPresence::Type GetUserPresence() = 0;

	/**
	 * Gets the current emulation mode. The default is no emulation.
	 *
	 * @return emulation mode.
	 */
	virtual EEyeXEmulationMode::Type GetEmulationMode() = 0;

	/**
	 * Sets the emulation mode for the eye-gaze point. The default is no emulation.
	 *
	 * @param Mode - emulation mode.
	 */
	virtual void SetEmulationMode(EEyeXEmulationMode::Type Mode) = 0;

};

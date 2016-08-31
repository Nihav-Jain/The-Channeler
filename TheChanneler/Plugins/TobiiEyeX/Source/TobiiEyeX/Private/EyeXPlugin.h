// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "IEyeXPlugin.h"
#include "EyeXClientLibraryLoader.h"
#include "EyeXDataStream.h"

class FEyeXDataStream;
class IEyeXStateAccessor;

/**
* The main plugin module of the EyeX plugin. Initializes the connection to the EyeX
* server and registers an event handler with the engine to handle events and store data.
* The public API functionality of the EyeX interface is implemented here.
*/
class FEyeXPlugin : public IEyeXPlugin
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
public:

	/** IEyeXPlugin implementation */
	virtual FEyeXGazePoint GetGazePoint(EEyeXGazePointDataMode::Type Mode = EEyeXGazePointDataMode::LightlyFiltered) override;
	virtual TEyeXMaybeValue<FVector> Get3DGazePoint(EEyeXGazePointDataMode::Type Mode) override;
	virtual FEyeXFixationDataPoint GetLastFixation(EEyeXFixationDataMode::Type Mode) override;
	virtual FEyeXFixationDataPoint GetOngoingFixation(EEyeXFixationDataMode::Type Mode) override;
	virtual FEyeXEyePosition GetEyePosition() override;
	virtual TEyeXMaybeValue<FRotator> GetHeadRotation() override;
	virtual TEyeXMaybeValue<FEyeXScreenBounds> GetScreenBounds() override;
	virtual TEyeXMaybeValue<FVector2D> GetDisplaySize() override;
	virtual EEyeXDeviceStatus::Type GetEyeTrackingDeviceStatus() override;
	virtual EEyeXUserPresence::Type GetUserPresence() override;
	virtual EEyeXEmulationMode::Type GetEmulationMode() override;
	virtual void SetEmulationMode(EEyeXEmulationMode::Type Mode) override;

private:

	void OnConnectionStateChanged(TX_CONNECTIONSTATE ConnectionState);
	void HandleEvent(TX_CONSTHANDLE AsyncData);

	const FEyeXGazePointDataStream *GetOrCreateGazePointDataStream(EEyeXGazePointDataMode::Type Mode);
	const FEyeXFixationDataStream *GetOrCreateFixationDataStream(EEyeXFixationDataMode::Type Mode);
	const FEyeXEyePositionDataStream *GetOrCreateEyePositionDataStream();
	void StopDataStreams();
	void StopStateAccessors();
	void SendGlobalInteractorSnapshot(bool markAsDeleted = false);
	void UpdateConnectionStateOnAllStateAccessors();

	FEyeXFixationDataPoint ConvertFixationPointToViewportPixels(FEyeXFixationDataPoint dataPointInVirtualDesktopPixels);

private:

	// NOTE: avoid object (that is, non-pointer) fields because we want to do as little initialization as possible before StartupModule.
	FEyeXClientLibraryLoader* EyeXClient;
	TX_CONTEXTHANDLE Context;
	bool bIsConnected;
	TArray<FEyeXDataStream*> LiveDataStreams;
	IEyeXStateAccessor *ScreenBoundsAccessor;
	IEyeXStateAccessor *DisplaySizeAccessor;
	IEyeXStateAccessor *EyeTrackingDeviceStatusAccessor;
	IEyeXStateAccessor *UserPresenceAccessor;
	EEyeXEmulationMode::Type EmulationMode;
};

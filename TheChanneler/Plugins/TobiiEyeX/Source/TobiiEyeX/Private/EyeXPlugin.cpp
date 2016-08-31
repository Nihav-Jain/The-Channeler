// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXPlugin.h"
#include "EyeXStateAccessor.h"
#include "EyeXUtils.h"

IMPLEMENT_MODULE(FEyeXPlugin, TobiiEyeX)

/*
 * Macros for uniform access to all state accessors.
 */

#define ALL_STATE_ACCESSORS IEyeXStateAccessor** allStateAccessors[] = \
	{ \
		&ScreenBoundsAccessor, \
		&DisplaySizeAccessor, \
		&EyeTrackingDeviceStatusAccessor, \
		&UserPresenceAccessor \
	}

#define STATE_ACCESSOR_COUNT sizeof(allStateAccessors) / sizeof(IEyeXStateAccessor**)


void FEyeXPlugin::StartupModule()
{
	UE_LOG(LogEyeX, Log, TEXT("Starting up TobiiEyeX Module."));

	// initialize to a clean state.
	EyeXClient = nullptr;
	Context = TX_EMPTY_HANDLE;
	bIsConnected = false;
	EmulationMode = EEyeXEmulationMode::Disabled;

	ALL_STATE_ACCESSORS;
	for (int i = 0; i < STATE_ACCESSOR_COUNT; i++)
	{
		*allStateAccessors[i] = nullptr;
	}

	if (!IsAvailable())
	{
		UE_LOG(LogEyeX, Log, TEXT("TobiiEyeX Module not available."));
		return;
	}

	// load the client library.
	EyeXClient = new FEyeXClientLibraryLoader();
	if (!EyeXClient->IsAvailable())
	{
		UE_LOG(LogEyeX, Error, TEXT("The EyeX client library could not be loaded."));
		return;
	}

	// initialize EyeX.
	TX_RESULT result = EyeXClient->InitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, nullptr, nullptr, nullptr, nullptr);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeX, Error, TEXT("Failed to initialize the EyeX environment. Error code: %d."), (int32)result);
		return;
	}

	result = EyeXClient->CreateContext(&Context, TX_FALSE);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeX, Error, TEXT("Failed to create context. Error code: %d."), (int32)result);
		return;
	}

	auto connectionStateChangedTrampoline = [](TX_CONNECTIONSTATE ConnectionState, TX_USERPARAM UserParam)
	{
		static_cast<FEyeXPlugin*>(UserParam)->OnConnectionStateChanged(ConnectionState);
	};

	TX_TICKET ConnectionStateChangedTicket = TX_INVALID_TICKET;
	result = EyeXClient->RegisterConnectionStateChangedHandler(Context, &ConnectionStateChangedTicket, connectionStateChangedTrampoline, this);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeX, Error, TEXT("Could not register connection state callback. Error code: %d."), (int32)result);
		return;
	}

	auto eventHandlerTrampoline = [](TX_CONSTHANDLE AsyncData, TX_USERPARAM UserParam)
	{
		static_cast<FEyeXPlugin*>(UserParam)->HandleEvent(AsyncData);
	};

	TX_TICKET EventHandlerTicket = TX_INVALID_TICKET;
	result = EyeXClient->RegisterEventHandler(Context, &EventHandlerTicket, eventHandlerTrampoline, this);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeX, Error, TEXT("Could not register event handler callback. Error code: %d."), (int32)result);
		return;
	}

	result = EyeXClient->EnableConnection(Context);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeX, Error, TEXT("Could not enable connection to the EyeX Engine. Error code: %d."), (int32)result);
	}
}

void FEyeXPlugin::ShutdownModule()
{
	UE_LOG(LogEyeX, Log, TEXT("Shutting down TobiiEyeX Module."));

	StopDataStreams();
	StopStateAccessors();

	if (Context != TX_EMPTY_HANDLE)
	{
		TX_RESULT result = EyeXClient->ShutdownContext(Context, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE);
		if (TX_RESULT_OK != result)
		{
			UE_LOG(LogEyeX, Error, TEXT("Context shutdown failed. Error code: %d."), (int32)result);
		}

		result = EyeXClient->ReleaseContext(&Context);
		if (TX_RESULT_OK != result)
		{
			UE_LOG(LogEyeX, Error, TEXT("Context could not be released. Error code: %d."), (int32)result);
		}
	}

	if (EyeXClient != nullptr)
	{
		if (EyeXClient->IsAvailable())
		{
			TX_RESULT result = EyeXClient->UninitializeEyeX();
			if (TX_RESULT_OK != result)
			{
				UE_LOG(LogEyeX, Error, TEXT("Environment shutdown failed. Error code: %d."), (int32)result);
			}
		}

		delete EyeXClient;
		EyeXClient = nullptr;
	}
}

FEyeXGazePoint FEyeXPlugin::GetGazePoint(EEyeXGazePointDataMode::Type Mode)
{
	if (EmulationMode == EEyeXEmulationMode::Enabled)
	{
		auto gameViewport = FEyeXUtils::GetGameViewport();
		if (gameViewport == nullptr)
		{
			return FEyeXGazePoint::Invalid();
		}

		FVector2D mousePosition;
		gameViewport->GetMousePosition(mousePosition);
		return FEyeXGazePoint(mousePosition, 0, true);
	}

	auto stream = GetOrCreateGazePointDataStream(Mode);
	auto last = stream->Last();
	auto screenBounds = GetScreenBounds();

	if (last.bHasValue && screenBounds.bHasValue)
	{
		auto viewportPoint = FEyeXUtils::VirtualDesktopPixelToViewportPixel(last.Value, screenBounds.Value);
		if (viewportPoint.bHasValue)
		{
			return FEyeXGazePoint(viewportPoint.Value, last.TimeStamp, true);
		}
	}

	return last;
}

TEyeXMaybeValue<FVector> FEyeXPlugin::Get3DGazePoint(EEyeXGazePointDataMode::Type Mode)
{
	auto stream = GetOrCreateGazePointDataStream(Mode);
	auto gazePointInVirtualDesktopPixels = stream->Last();

	if (gazePointInVirtualDesktopPixels.bHasValue)
	{
		auto displaySize = GetDisplaySize();
		auto screenBounds = GetScreenBounds();

		if (displaySize.bHasValue && screenBounds.bHasValue)
		{
			auto gazePointEyeX3D = FEyeXUtils::VirtualDesktopPixelToEyeX3DPoint(gazePointInVirtualDesktopPixels.Value, displaySize.Value, screenBounds.Value);
			auto gazePointUnrealWorld = FEyeXUtils::EyeX3DPointToUnrealWorld(gazePointEyeX3D);
			return TEyeXMaybeValue<FVector>(gazePointUnrealWorld);
		}
	}

	return TEyeXMaybeValue<FVector>(FVector::ZeroVector, false);
}

FEyeXFixationDataPoint FEyeXPlugin::GetLastFixation(EEyeXFixationDataMode::Type Mode)
{
	auto stream = GetOrCreateFixationDataStream(Mode);
	return ConvertFixationPointToViewportPixels(stream->Last());
}

FEyeXFixationDataPoint FEyeXPlugin::GetOngoingFixation(EEyeXFixationDataMode::Type Mode)
{
	auto stream = GetOrCreateFixationDataStream(Mode);
	return ConvertFixationPointToViewportPixels(stream->Ongoing());
}

FEyeXFixationDataPoint FEyeXPlugin::ConvertFixationPointToViewportPixels(FEyeXFixationDataPoint fixation)
{
	auto screenBounds = GetScreenBounds();

	if (fixation.bHasValue && screenBounds.bHasValue)
	{
		auto viewportPoint = FEyeXUtils::VirtualDesktopPixelToViewportPixel(fixation.GazePoint, screenBounds.Value);
		if (viewportPoint.bHasValue)
		{
			return FEyeXFixationDataPoint(viewportPoint.Value, fixation.BeginTimeStamp, fixation.Duration, true);
		}
	}

	return FEyeXFixationDataPoint::Invalid();
}

FEyeXEyePosition FEyeXPlugin::GetEyePosition()
{
	auto stream = GetOrCreateEyePositionDataStream();
	auto eyePosition = stream->Last();

	if (eyePosition.bIsLeftEyeValid)
	{
		eyePosition.LeftEye = FEyeXUtils::EyeX3DPointToUnrealWorld(eyePosition.LeftEye);
	}

	if (eyePosition.bIsRightEyeValid)
	{
		eyePosition.RightEye = FEyeXUtils::EyeX3DPointToUnrealWorld(eyePosition.RightEye);
	}

	return eyePosition;
}

TEyeXMaybeValue<FRotator> FEyeXPlugin::GetHeadRotation()
{
	auto eyePosition = GetEyePosition();
	if (!eyePosition.bIsLeftEyeValid || !eyePosition.bIsRightEyeValid)
	{
		return TEyeXMaybeValue<FRotator>(FRotator::ZeroRotator, false);
	}

	const float Pitch = 0.0f;
	const float Yaw = FEyeXUtils::CalculateRotationAngle(FVector2D(eyePosition.LeftEye.Y, eyePosition.LeftEye.X), FVector2D(eyePosition.RightEye.Y, eyePosition.RightEye.X));
	const float Roll = FEyeXUtils::CalculateRotationAngle(FVector2D(eyePosition.LeftEye.Y, eyePosition.LeftEye.Z), FVector2D(eyePosition.RightEye.Y, eyePosition.RightEye.Z));
	return FRotator(Pitch, Yaw, Roll);
}

TEyeXMaybeValue<FEyeXScreenBounds> FEyeXPlugin::GetScreenBounds()
{
	if (ScreenBoundsAccessor == nullptr)
	{
		ScreenBoundsAccessor = new TEyeXStateAccessor<TX_RECT>(Context, bIsConnected, TX_STATEPATH_SCREENBOUNDS, *EyeXClient);
	}

	if (!ScreenBoundsAccessor->bHasValue())
	{
		return TEyeXMaybeValue<FEyeXScreenBounds>(FEyeXScreenBounds::Invalid(), false);
	}

	auto& value = ((TEyeXStateAccessor<TX_RECT>*)ScreenBoundsAccessor)->Value();
	return TEyeXMaybeValue<FEyeXScreenBounds>(FEyeXScreenBounds((int32)value.X, (int32)value.Y, (int32)value.Width, (int32)value.Height));
}

TEyeXMaybeValue<FVector2D> FEyeXPlugin::GetDisplaySize()
{
	if (DisplaySizeAccessor == nullptr)
	{
		DisplaySizeAccessor = new TEyeXStateAccessor<TX_SIZE2>(Context, bIsConnected, TX_STATEPATH_DISPLAYSIZE, *EyeXClient);
	}

	if (!DisplaySizeAccessor->bHasValue())
	{
		return TEyeXMaybeValue<FVector2D>(FVector2D::ZeroVector, false);
	}

	auto& value = ((TEyeXStateAccessor<TX_SIZE2>*)DisplaySizeAccessor)->Value();
	return TEyeXMaybeValue<FVector2D>(FVector2D((float)value.Width, (float)value.Height));
}

EEyeXDeviceStatus::Type FEyeXPlugin::GetEyeTrackingDeviceStatus()
{
	if (EyeTrackingDeviceStatusAccessor == nullptr)
	{
		EyeTrackingDeviceStatusAccessor = new TEyeXStateAccessor<TX_INTEGER>(Context, bIsConnected, TX_STATEPATH_EYETRACKINGSTATE, *EyeXClient);
	}

	if (!EyeTrackingDeviceStatusAccessor->bHasValue())
	{
		return EEyeXDeviceStatus::Unknown;
	}

	auto& value = ((TEyeXStateAccessor<TX_INTEGER>*)EyeTrackingDeviceStatusAccessor)->Value();
	switch (value)
	{
	case TX_EYETRACKINGDEVICESTATUS_INITIALIZING:
	case TX_EYETRACKINGDEVICESTATUS_CONFIGURING:
		return EEyeXDeviceStatus::Pending;

	case TX_EYETRACKINGDEVICESTATUS_TRACKING:
		return EEyeXDeviceStatus::Tracking;

	case TX_EYETRACKINGDEVICESTATUS_TRACKINGPAUSED:
		return EEyeXDeviceStatus::Disabled;

	default:
		break;
	}

	return EEyeXDeviceStatus::NotAvailable;
}

EEyeXUserPresence::Type FEyeXPlugin::GetUserPresence()
{
	if (UserPresenceAccessor == nullptr)
	{
		UserPresenceAccessor = new TEyeXStateAccessor<TX_INTEGER>(Context, bIsConnected, TX_STATEPATH_USERPRESENCE, *EyeXClient);
	}

	if (!UserPresenceAccessor->bHasValue())
	{
		return EEyeXUserPresence::Unknown;
	}

	auto& value = ((TEyeXStateAccessor<TX_INTEGER>*)UserPresenceAccessor)->Value();
	return (value == TX_USERPRESENCE_PRESENT) ? EEyeXUserPresence::Present : EEyeXUserPresence::NotPresent;
}

EEyeXEmulationMode::Type FEyeXPlugin::GetEmulationMode()
{
	return EmulationMode;
}

void FEyeXPlugin::SetEmulationMode(EEyeXEmulationMode::Type Mode)
{
	EmulationMode = Mode;
}

void FEyeXPlugin::OnConnectionStateChanged(TX_CONNECTIONSTATE ConnectionState)
{
	switch (ConnectionState)
	{
	case TX_CONNECTIONSTATE_CONNECTED:
		UE_LOG(LogEyeX, Log, TEXT("Connected to the EyeX Engine."));
		bIsConnected = true;
		SendGlobalInteractorSnapshot();
		break;

	default:
		bIsConnected = false;
		break;
	}

	UpdateConnectionStateOnAllStateAccessors();
}

void FEyeXPlugin::HandleEvent(TX_CONSTHANDLE AsyncData)
{
	FEyeXScopedHandle event(*EyeXClient);
	if (TX_RESULT_OK != EyeXClient->GetAsyncDataContent(AsyncData, &event.Handle))
	{
		return;
	}

	// extract the interactor ID from the event
	char interactorId[MAX_DATA_STREAM_INTERACTOR_ID_SIZE];
	int size = MAX_DATA_STREAM_INTERACTOR_ID_SIZE;
	if (TX_RESULT_OK != EyeXClient->GetEventInteractorId(event.Handle, interactorId, &size))
	{
		return;
	}

	// route the event to the proper data stream based on the interactor ID
	for (auto stream : LiveDataStreams)
	{
		if (0 == strcmp(interactorId, stream->GetInteractorId()))
		{
			stream->HandleEvent(event.Handle);
			break;
		}
	}
}

const FEyeXGazePointDataStream *FEyeXPlugin::GetOrCreateGazePointDataStream(EEyeXGazePointDataMode::Type Mode)
{
	char interactorId[MAX_DATA_STREAM_INTERACTOR_ID_SIZE];
	FEyeXGazePointDataStream::GetInteractorId(interactorId, Mode);
	for (auto stream : LiveDataStreams)
	{
		if (0 == strcmp(interactorId, stream->GetInteractorId()))
		{
			return static_cast<const FEyeXGazePointDataStream*>(stream);
		}
	}

	auto stream = new FEyeXGazePointDataStream(Mode, *EyeXClient);
	LiveDataStreams.Add(stream);
	SendGlobalInteractorSnapshot();
	return stream;
}

const FEyeXFixationDataStream *FEyeXPlugin::GetOrCreateFixationDataStream(EEyeXFixationDataMode::Type Mode)
{
	char interactorId[MAX_DATA_STREAM_INTERACTOR_ID_SIZE];
	FEyeXFixationDataStream::GetInteractorId(interactorId, Mode);
	for (auto stream : LiveDataStreams)
	{
		if (0 == strcmp(interactorId, stream->GetInteractorId()))
		{
			return static_cast<const FEyeXFixationDataStream*>(stream);
		}
	}

	auto stream = new FEyeXFixationDataStream(Mode, *EyeXClient);
	LiveDataStreams.Add(stream);
	SendGlobalInteractorSnapshot();
	return stream;
}

const FEyeXEyePositionDataStream *FEyeXPlugin::GetOrCreateEyePositionDataStream()
{
	char interactorId[MAX_DATA_STREAM_INTERACTOR_ID_SIZE];
	FEyeXEyePositionDataStream::GetInteractorId(interactorId);
	for (auto stream : LiveDataStreams)
	{
		if (0 == strcmp(interactorId, stream->GetInteractorId()))
		{
			return static_cast<const FEyeXEyePositionDataStream*>(stream);
		}
	}

	auto stream = new FEyeXEyePositionDataStream(*EyeXClient);
	LiveDataStreams.Add(stream);
	SendGlobalInteractorSnapshot();
	return stream;
}

void FEyeXPlugin::StopDataStreams()
{
	SendGlobalInteractorSnapshot(true);

	for (auto stream : LiveDataStreams)
	{
		delete stream;
	}

	LiveDataStreams.Reset();
}

void FEyeXPlugin::StopStateAccessors()
{
	ALL_STATE_ACCESSORS;
	for (int i = 0; i < STATE_ACCESSOR_COUNT; i++)
	{
		if (*allStateAccessors[i] != nullptr)
		{
			delete *allStateAccessors[i];
			*allStateAccessors[i] = nullptr;
		}
	}
}

void FEyeXPlugin::SendGlobalInteractorSnapshot(bool markAsDeleted)
{
	if (!bIsConnected || 
		LiveDataStreams.Num() == 0)
	{
		return;
	}

	FEyeXScopedHandle snapshot(*EyeXClient);
	EyeXClient->CreateSnapshot(Context, &snapshot.Handle);
	EyeXClient->AddSnapshotWindowId(snapshot.Handle, TX_LITERAL_GLOBALINTERACTORWINDOWID);

	FEyeXScopedHandle bounds(*EyeXClient);
	EyeXClient->CreateSnapshotBounds(snapshot.Handle, &bounds.Handle, TX_BOUNDSTYPE_NONE);

	for (auto stream : LiveDataStreams)
	{
		stream->AddToSnapshot(snapshot.Handle, markAsDeleted);
	}

	EyeXClient->CommitSnapshotAsync(snapshot.Handle, nullptr, nullptr);
}

void FEyeXPlugin::UpdateConnectionStateOnAllStateAccessors()
{
	ALL_STATE_ACCESSORS;
	for (int i = 0; i < STATE_ACCESSOR_COUNT; i++)
	{
		if (*allStateAccessors[i] != nullptr)
		{
			if (bIsConnected)
			{
				(*allStateAccessors[i])->OnConnected();
			}
			else
			{
				(*allStateAccessors[i])->OnDisconnected();
			}
		}
	}
}

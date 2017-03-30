#include "TheChanneler.h"
#include "IEyeXPlugin.h"
#include "EyeXPluginEx.h"

DEFINE_LOG_CATEGORY(LogEyeXEx);

#if !defined(EYEX_DLL_IMPORT)
#define EYEX_DLL_IMPORT(FunctionName) {															\
		const FString EntryPoint = (FString)"tx" + (FString)#FunctionName;							\
		FunctionName = (FunctionName##Hook)FPlatformProcess::GetDllExport(DllHandle, *EntryPoint);	\
		if (!FunctionName) {																		\
			UE_LOG(LogEyeXEx, Error, TEXT("Failed to import DLL function %s!"), *EntryPoint);			\
			}																						\
		} 
#endif

UEyeXPluginEx* UEyeXPluginEx::Instance;

const std::string UEyeXPluginEx::EyeXeyePositionInteractor = "EyeX_eye_position";

UEyeXPluginEx::UEyeXPluginEx() :
	Plugin(&IEyeXPlugin::Get()),
	LaunchCalibrationToolCallback(nullptr),
	bConnected(false),
	bInConfiguration(false),
	XThresholod(0.1f),
	YThresholod(0.2f),
	ZThresholod(0.2f)
{
	const FString DLL_PATH = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		*FPaths::GamePluginsDir(),
		TEXT("TobiiEyeX/ThirdParty/EyeX/lib/"),
		TEXT("x64"),
		TEXT("Tobii.EyeX.Client.dll")));

	DllHandle = FPlatformProcess::GetDllHandle(*DLL_PATH);
	if (!DllHandle)
	{
		UE_LOG(LogEyeXEx, Error, TEXT("Tobii.EyeX.Client.dll missing from %s !"), *DLL_PATH);
		return;
	}

	EYEX_DLL_IMPORT(CreateContext);
	EYEX_DLL_IMPORT(ReleaseContext);
	EYEX_DLL_IMPORT(ShutdownContext);
	EYEX_DLL_IMPORT(EnableConnection);

	EYEX_DLL_IMPORT(RegisterConnectionStateChangedHandler);
	EYEX_DLL_IMPORT(UnregisterConnectionStateChangedHandler);

	EYEX_DLL_IMPORT(GetState);
	EYEX_DLL_IMPORT(GetStateValueAsString);
	EYEX_DLL_IMPORT(GetAsyncDataContent);
	EYEX_DLL_IMPORT(GetStateValueAsInteger);
	EYEX_DLL_IMPORT(SetCurrentProfile);
	EYEX_DLL_IMPORT(ReleaseObject);
	EYEX_DLL_IMPORT(LaunchConfigurationTool);
	EYEX_DLL_IMPORT(RegisterStateChangedHandler);
	EYEX_DLL_IMPORT(GetEyePositionDataEventParams);
	EYEX_DLL_IMPORT(RegisterEventHandler);

	EYEX_DLL_IMPORT(CreateSnapshot);
	EYEX_DLL_IMPORT(AddSnapshotWindowId);
	EYEX_DLL_IMPORT(CreateSnapshotBounds);
	EYEX_DLL_IMPORT(CreateInteractor);
	EYEX_DLL_IMPORT(CreateInteractorBounds);
	EYEX_DLL_IMPORT(CreateInteractorBehavior);
	EYEX_DLL_IMPORT(SetInteractorDeleted);
	EYEX_DLL_IMPORT(CommitSnapshotAsync);
	EYEX_DLL_IMPORT(GetEventInteractorId);
	EYEX_DLL_IMPORT(GetEventBehavior);

	// initialize EyeX.
	TX_RESULT result = CreateContext(&Context, TX_FALSE);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeXEx, Error, TEXT("Failed to create context. Error code: %d."), (int32)result);
		return;
	}

	auto connectionStateChangedTrampoline = [](TX_CONNECTIONSTATE ConnectionState, TX_USERPARAM UserParam)
	{
		static_cast<UEyeXPluginEx*>(UserParam)->OnConnectionStateChanged(ConnectionState);
	};

	TX_TICKET ConnectionStateChangedTicket = TX_INVALID_TICKET;
	result = RegisterConnectionStateChangedHandler(Context, &ConnectionStateChangedTicket, connectionStateChangedTrampoline, this);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeXEx, Error, TEXT("Could not register connection state callback. Error code: %d."), (int32)result);
		return;
	}

	result = EnableConnection(Context);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeXEx, Error, TEXT("Could not enable connection to the EyeX Engine. Error code: %d."), (int32)result);
	}

	TX_TICKET DeviceStatusChangedTicket = TX_INVALID_TICKET;
	auto deviceStatusChangedLambda = [](TX_CONSTHANDLE AsyncData, TX_USERPARAM UserParam)
	{
		static_cast<UEyeXPluginEx*>(UserParam)->OnDeviceStatusChanged(AsyncData);
	};
	result = RegisterStateChangedHandler(Context, &DeviceStatusChangedTicket,
		TX_STATEPATH_EYETRACKINGSTATE, deviceStatusChangedLambda, this);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeXEx, Error, TEXT("Could not register device status callback. Error code: %d."), (int32)result);
		return;
	}

	auto eventHandlerTrampoline = [](TX_CONSTHANDLE AsyncData, TX_USERPARAM UserParam)
	{
		static_cast<UEyeXPluginEx*>(UserParam)->HandleEvent(AsyncData);
	};

	TX_TICKET EventHandlerTicket = TX_INVALID_TICKET;
	result = RegisterEventHandler(Context, &EventHandlerTicket, eventHandlerTrampoline, this);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeXEx, Error, TEXT("Could not register event handler callback. Error code: %d."), (int32)result);
		return;
	}
}

UEyeXPluginEx::~UEyeXPluginEx()
{
	if (Context != TX_EMPTY_HANDLE)
	{
		SetupEyePositionInteractor(true);

		TX_RESULT result = ShutdownContext(Context, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE);
		if (TX_RESULT_OK != result)
		{
			UE_LOG(LogEyeXEx, Error, TEXT("Context shutdown failed. Error code: %d."), (int32)result);
		}

		result = ReleaseContext(&Context);
		if (TX_RESULT_OK != result)
		{
			UE_LOG(LogEyeXEx, Error, TEXT("Context could not be released. Error code: %d."), (int32)result);
		}
	}

	if (DllHandle)
	{
		FPlatformProcess::FreeDllHandle(DllHandle);
	}

	if (LaunchCalibrationToolCallback) {
		delete LaunchCalibrationToolCallback;
		LaunchCalibrationToolCallback = nullptr;
	}
}


FEyeXGazePoint UEyeXPluginEx::GetGazePoint(EEyeXGazePointDataMode::Type Mode)
{
	return Plugin->GetGazePoint(Mode);
}

TEyeXMaybeValue<FVector> UEyeXPluginEx::Get3DGazePoint(EEyeXGazePointDataMode::Type Mode)
{
	return Plugin->Get3DGazePoint(Mode);
}

FEyeXFixationDataPoint UEyeXPluginEx::GetLastFixation(EEyeXFixationDataMode::Type Mode)
{
	return Plugin->GetLastFixation(Mode);
}

FEyeXFixationDataPoint UEyeXPluginEx::GetOngoingFixation(EEyeXFixationDataMode::Type Mode)
{
	return Plugin->GetOngoingFixation(Mode);
}

FEyeXEyePosition UEyeXPluginEx::GetEyePosition()
{
	return Plugin->GetEyePosition();
}

TEyeXMaybeValue<FRotator> UEyeXPluginEx::GetHeadRotation()
{
	return Plugin->GetHeadRotation();
}

TEyeXMaybeValue<FEyeXScreenBounds> UEyeXPluginEx::GetScreenBounds()
{
	return Plugin->GetScreenBounds();
}

TEyeXMaybeValue<FVector2D> UEyeXPluginEx::GetDisplaySize()
{
	return Plugin->GetDisplaySize();
}

EEyeXDeviceStatus::Type UEyeXPluginEx::GetEyeTrackingDeviceStatus()
{
	return Plugin->GetEyeTrackingDeviceStatus();
}

EEyeXUserPresence::Type UEyeXPluginEx::GetUserPresence()
{
	return Plugin->GetUserPresence();
}

EEyeXEmulationMode::Type UEyeXPluginEx::GetEmulationMode()
{
	return Plugin->GetEmulationMode();
}

void UEyeXPluginEx::SetEmulationMode(EEyeXEmulationMode::Type Mode)
{
	Plugin->SetEmulationMode(Mode);
}

void UEyeXPluginEx::SetProfile(const FString & name)
{
	TX_RESULT result = SetCurrentProfile(Context, TCHAR_TO_UTF8(*name), nullptr, nullptr);
}

FString UEyeXPluginEx::GetProfileName()
{
	FString str;

	if (bConnected) {
		TX_CHAR buf[256] = { 0 };
		TX_SIZE size = 256;
		TX_HANDLE StateBag = TX_EMPTY_HANDLE;
		TX_RESULT result = GetState(Context, TX_STATEPATH_EYETRACKINGCURRENTPROFILENAME, &StateBag);
		if (TX_RESULT_OK == result)
		{
			result = GetStateValueAsString(StateBag, TX_STATEPATH_EYETRACKINGCURRENTPROFILENAME, buf, &size);
			if (TX_RESULT_OK == result)
			{
				str = FString(buf);
			}
			else {
				UE_LOG(LogEyeXEx, Error, TEXT("Could not GetStateValueAsString. Error code: %d."), (int32)result);
			}
		}
		else {
			UE_LOG(LogEyeXEx, Error, TEXT("Could not GetState. Error code: %d."), (int32)result);
		}

		if (StateBag != TX_EMPTY_HANDLE) {
			ReleaseObject(&StateBag);
		}
	}

	return str;
}

TArray<FString> UEyeXPluginEx::GetProfiles()
{
	TArray<FString> names;

	if (bConnected) {
		TX_CHAR buf[256] = { 0 };
		TX_SIZE size = 256;
		TX_HANDLE StateBag = TX_EMPTY_HANDLE;
		TX_RESULT result = GetState(Context, TX_STATEPATH_EYETRACKINGPROFILES, &StateBag);
		if (TX_RESULT_OK == result)
		{
			result = GetStateValueAsString(StateBag, TX_STATEPATH_EYETRACKINGPROFILES, buf, &size);
			if (TX_RESULT_OK == result)
			{
				// Loop throught all names
				for (int32 i = 0; i < size; ++i) {
					if (i == 0 || buf[i - 1] == 0) {
						if (buf[i] == 0) {
							break;
						}
						else {
							names.Emplace(&buf[i], nullptr);
						}
					}
				}
			}
			else {
				UE_LOG(LogEyeXEx, Error, TEXT("Could not GetStateValueAsString. Error code: %d."), (int32)result);
			}
		}
		else {
			UE_LOG(LogEyeXEx, Error, TEXT("Could not GetState. Error code: %d."), (int32)result);
		}

		if (StateBag != TX_EMPTY_HANDLE) {
			ReleaseObject(&StateBag);
		}
	}

	return names;
}


void UEyeXPluginEx::OnConnectionStateChanged(TX_CONNECTIONSTATE ConnectionState)
{
	switch (ConnectionState)
	{
	case TX_CONNECTIONSTATE_CONNECTED:
		UE_LOG(LogEyeXEx, Log, TEXT("Connected to the EyeX Engine."));
		SetupEyePositionInteractor(false);
		bConnected = true;
		break;
	default:
		bConnected = false;
		break;
	}
}

void UEyeXPluginEx::LaunchCalibrationTool(std::function<void(int)> callback)
{
	if (bConnected) {
		LaunchCalibrationToolCallback = new std::function<void(int)>(callback);
		UE_LOG(LogEyeXEx, Warning, TEXT("Launching calibration tool."));
		TX_RESULT result =  LaunchConfigurationTool(Context, TX_CONFIGURATIONTOOL_RECALIBRATE, NULL, NULL);
		if (TX_RESULT_OK != result)
		{
			UE_LOG(LogEyeXEx, Error, TEXT("Could not launch tool. Error code: %d."), (int32)result);
			return;
		}
	}
}

void UEyeXPluginEx::OnDeviceStatusChanged(TX_CONSTHANDLE asyncData)
{
	if (bConnected) {
		FScopeLock lock(&mutex);
		TX_HANDLE StateBag = TX_EMPTY_HANDLE;
		TX_INTEGER statusResult = TX_EYETRACKINGDEVICESTATUS_UNKNOWNERROR;
		TX_RESULT result = GetAsyncDataContent(asyncData, &StateBag);
		if (TX_RESULT_OK == result)
		{
			result = GetStateValueAsInteger(StateBag, TX_STATEPATH_EYETRACKINGSTATE, &statusResult);
			if (TX_RESULT_OK != result)
			{
				UE_LOG(LogEyeXEx, Error, TEXT("Could not GetStateValueAsInteger. Error code: %d."), (int32)result);
			}
			else if (!bInConfiguration && statusResult == TX_EYETRACKINGDEVICESTATUS_CONFIGURING) {
				bInConfiguration = true;
			}
			UE_LOG(LogEyeXEx, Warning, TEXT("TX_STATEPATH_EYETRACKINGSTATE result = %d"), (int32)statusResult);
		}
		else {
			UE_LOG(LogEyeXEx, Error, TEXT("Could not GetAsyncDataContent. Error code: %d."), (int32)result);
		}

		if (StateBag != TX_EMPTY_HANDLE) {
			ReleaseObject(&StateBag);
		}

		if (bInConfiguration && statusResult == TX_EYETRACKINGDEVICESTATUS_TRACKING) {
			if (LaunchCalibrationToolCallback != nullptr) {
				UE_LOG(LogEyeXEx, Warning, TEXT("LaunchCalibrationToolCallback is not null."));
				auto ViewportClient = GEngine->GameViewport;
				if (ViewportClient != nullptr &&
					ViewportClient->GetWindow().Get() != nullptr) {
					ViewportClient->GetWindow()->BringToFront();
					UE_LOG(LogEyeXEx, Warning, TEXT("Trying to bring app to front."));
					// Win32
					INPUT inputStructure;
					inputStructure.ki.wVk = VK_SPACE;
					inputStructure.ki.wScan = 0;
					inputStructure.ki.dwFlags = 0;
					inputStructure.ki.time = 0;
					inputStructure.ki.dwExtraInfo = 0;
					inputStructure.type = INPUT_KEYBOARD;

					SendInput(1, &inputStructure, sizeof(INPUT));
					// Win32
				}

 				TGraphTask<FGameThreadCallbackTask<std::function<void(int)>, int>>::CreateTask(NULL, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(std::move(*LaunchCalibrationToolCallback), std::move(statusResult));

				delete LaunchCalibrationToolCallback;
				LaunchCalibrationToolCallback = nullptr;
			}

			bInConfiguration = false;
		}
	}
}

void UEyeXPluginEx::HandleEvent(TX_CONSTHANDLE AsyncData)
{
	TX_HANDLE Handle = TX_EMPTY_HANDLE;
	TX_HANDLE behavior = TX_EMPTY_HANDLE;
	if (TX_RESULT_OK == GetAsyncDataContent(AsyncData, &Handle))
	{
		char interactorId[20];
		int size = 20;
		if (TX_RESULT_OK == GetEventInteractorId(Handle, interactorId, &size))
		{
			if (0 == strcmp(interactorId, EyeXeyePositionInteractor.c_str()))
			{
				if (TX_RESULT_OK == GetEventBehavior(Handle, &behavior, TX_BEHAVIORTYPE_EYEPOSITIONDATA))
				{
					TX_EYEPOSITIONDATAEVENTPARAMS eventParams;
					if (TX_RESULT_OK == GetEyePositionDataEventParams(behavior, &eventParams))
					{
						TX_REAL* normalizedPos = &eventParams.LeftEyeXNormalized;
						for (int i = 0; i < 2; ++i) {
							if (!eventParams.HasLeftEyePosition && i == 0) {
								normalizedPos = &eventParams.RightEyeXNormalized;
								continue;
							}
							if (!eventParams.HasRightEyePosition && i == 1) {
								continue;
							}

							if (1.0 - normalizedPos[0] < YThresholod) {
								OnEyesMightLoseTracking.Broadcast(EEyesMightLoseTrackingType::Type::Y_TOO_LEFT);
							}
							else if (normalizedPos[0] - 0.0 < YThresholod){
								OnEyesMightLoseTracking.Broadcast(EEyesMightLoseTrackingType::Type::Y_TOO_RIGHT);
							}

							if (1.0 - normalizedPos[1] < ZThresholod) {
								OnEyesMightLoseTracking.Broadcast(EEyesMightLoseTrackingType::Type::Z_TOO_LOW);
							}
							else if (normalizedPos[1] - 0.0 < ZThresholod) {
								OnEyesMightLoseTracking.Broadcast(EEyesMightLoseTrackingType::Type::Z_TOO_HIGH);
							}

							if (1.0 - normalizedPos[2] < XThresholod) {
								OnEyesMightLoseTracking.Broadcast(EEyesMightLoseTrackingType::Type::X_TOO_FAR);
							}
							else if (normalizedPos[2] - 0.0 < XThresholod) {
								OnEyesMightLoseTracking.Broadcast(EEyesMightLoseTrackingType::Type::X_TOO_CLOSE);
							}
							
							normalizedPos = &eventParams.RightEyeXNormalized;
						}
					}
				}
			}
		}
	}

	if (Handle != TX_EMPTY_HANDLE)
	{
		ReleaseObject(&Handle);
	}
	if (behavior != TX_EMPTY_HANDLE)
	{
		ReleaseObject(&behavior);
	}
}

void UEyeXPluginEx::SetupEyePositionInteractor(bool makedDelete)
{
	TX_HANDLE snapshot = TX_EMPTY_HANDLE;
	CreateSnapshot(Context, &snapshot);
	AddSnapshotWindowId(snapshot, TX_LITERAL_GLOBALINTERACTORWINDOWID);

	TX_HANDLE bounds = TX_EMPTY_HANDLE;
	CreateSnapshotBounds(snapshot, &bounds, TX_BOUNDSTYPE_NONE);

	TX_HANDLE interactor = TX_EMPTY_HANDLE;
	CreateInteractor(snapshot, &interactor, EyeXeyePositionInteractor.c_str(), TX_LITERAL_ROOTID, TX_LITERAL_GLOBALINTERACTORWINDOWID);

	TX_HANDLE interactorbounds = TX_EMPTY_HANDLE;
	CreateInteractorBounds(interactor, &interactorbounds, TX_BOUNDSTYPE_NONE);

	TX_HANDLE behavior = TX_EMPTY_HANDLE;
	CreateInteractorBehavior(interactor, &behavior, TX_BEHAVIORTYPE_EYEPOSITIONDATA);

	if (makedDelete) {
		SetInteractorDeleted(interactor, TX_TRUE);
	}

	CommitSnapshotAsync(snapshot, nullptr, nullptr);

	if (snapshot != TX_EMPTY_HANDLE) {
		ReleaseObject(&snapshot);
	}
	if (bounds != TX_EMPTY_HANDLE) {
		ReleaseObject(&snapshot);
	}
	if (interactor != TX_EMPTY_HANDLE) {
		ReleaseObject(&snapshot);
	}
	if (interactorbounds != TX_EMPTY_HANDLE) {
		ReleaseObject(&snapshot);
	}
	if (behavior != TX_EMPTY_HANDLE) {
		ReleaseObject(&snapshot);
	}
}


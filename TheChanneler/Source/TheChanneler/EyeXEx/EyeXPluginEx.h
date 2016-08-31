#pragma once
#include "IEyeXPlugin.h"
#include <functional>
#define TX_NODEBUGOBJECT
#include "eyex/EyeX.h"
#include "EyeXPluginEx.generated.h"

#if !defined(EYEX_DLL_HOOK)
#define EYEX_DLL_HOOK(FunctionName) FunctionName##Hook FunctionName
#endif


DECLARE_LOG_CATEGORY_EXTERN(LogEyeXEx, Log, All);

UENUM(BlueprintType)
namespace EEyesMightLoseTrackingType
{
	enum Type
	{
		X_TOO_CLOSE			UMETA(DisplayName = "X_TOO_CLOSE"),
		X_TOO_FAR			UMETA(DisplayName = "X_TOO_FAR"),
		Y_TOO_RIGHT			UMETA(DisplayName = "Y_TOO_RIGHT"),
		Y_TOO_LEFT			UMETA(DisplayName = "Y_TOO_LEFT"),
		Z_TOO_HIGH			UMETA(DisplayName = "Z_TOO_HIGH"),
		Z_TOO_LOW			UMETA(DisplayName = "Z_TOO_LOW")
	};
}


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEyesMightLoseTrackingEvent, EEyesMightLoseTrackingType::Type, Type);

/**
 * The extented APIs of IEyeXPlugin
 */
UCLASS()
class THECHANNELER_API UEyeXPluginEx : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Wrapper singlton function, this returns UEyeXPluginEx reference instead of IEyeXPlugin
	 * @see IEyeXPlugin.h
	 */
	static inline UEyeXPluginEx& Get()
	{
		if (Instance == nullptr) {
			// TODO: Check memleak
			Instance = NewObject<UEyeXPluginEx>(UEyeXPluginEx::StaticClass(), TEXT("EyeXex"), RF_RootSet);
		}

		return *Instance;
	}

	/**
	* Wrapper API functions
	* @see IEyeXPlugin.h
	*/

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("TobiiEyeX");
	}

	FEyeXGazePoint GetGazePoint(EEyeXGazePointDataMode::Type Mode = EEyeXGazePointDataMode::LightlyFiltered);

	TEyeXMaybeValue<FVector> Get3DGazePoint(EEyeXGazePointDataMode::Type Mode);

	FEyeXFixationDataPoint GetLastFixation(EEyeXFixationDataMode::Type Mode);

	FEyeXFixationDataPoint GetOngoingFixation(EEyeXFixationDataMode::Type Mode);

	FEyeXEyePosition GetEyePosition();

	TEyeXMaybeValue<FRotator> GetHeadRotation();

	TEyeXMaybeValue<FEyeXScreenBounds> GetScreenBounds();

	TEyeXMaybeValue<FVector2D> GetDisplaySize();

	EEyeXDeviceStatus::Type GetEyeTrackingDeviceStatus();

	EEyeXUserPresence::Type GetUserPresence();

	EEyeXEmulationMode::Type GetEmulationMode();

	void SetEmulationMode(EEyeXEmulationMode::Type Mode);

	/**
	* Set the current profile
	* @param name the profile name
	*/
	void SetProfile(const FString & name);

	/**
	* Get the profile name that is currently using
	* NOTE: This is now a sync function, don't call it if you need async behavior
	* @return the profile name
	*/
	FString GetProfileName();

	/**
	* Get all profiles
	* NOTE: This is now a sync function, don't call it if you need async behavior
	* @return the list of profile name
	*/
	TArray<FString> GetProfiles();

	/**
	* Launch calibration tool to create a new profile
	* This function is not thread-safe yet
	* @param callback The callback function when the calibration is done, 
	*	it pass the int result
	*/
	void LaunchCalibrationTool(std::function<void(int)> callback);

	/**
	* The event that fires when the eye is about to leave the tracking frustum
	*/
	FOnEyesMightLoseTrackingEvent OnEyesMightLoseTracking;

	/**
	* Set the thresholod for how often the event should be fired.
	* @param x 0 doesn't fire, 1 always fires
	* @param y 0 doesn't fire, 1 always fires
	* @param z 0 doesn't fire, 1 always fires
	*/
	void SetMightLoseTrackingThreshold(float x, float y, float z);

private:
	UEyeXPluginEx();
	~UEyeXPluginEx();

	static UPROPERTY() UEyeXPluginEx* Instance;

	IEyeXPlugin* Plugin;
	void* DllHandle;

	EYEX_DLL_HOOK(CreateContext);
	EYEX_DLL_HOOK(ReleaseContext);
	EYEX_DLL_HOOK(ShutdownContext);
	EYEX_DLL_HOOK(EnableConnection);

	EYEX_DLL_HOOK(RegisterConnectionStateChangedHandler);
	EYEX_DLL_HOOK(UnregisterConnectionStateChangedHandler);

	EYEX_DLL_HOOK(GetState);
	EYEX_DLL_HOOK(GetStateValueAsString);
	EYEX_DLL_HOOK(GetAsyncDataContent);
	EYEX_DLL_HOOK(GetStateValueAsInteger);
	EYEX_DLL_HOOK(SetCurrentProfile);
	EYEX_DLL_HOOK(ReleaseObject);
	EYEX_DLL_HOOK(LaunchConfigurationTool);
	EYEX_DLL_HOOK(RegisterStateChangedHandler);
	EYEX_DLL_HOOK(GetEyePositionDataEventParams);
	EYEX_DLL_HOOK(GetEventBehavior);
	EYEX_DLL_HOOK(RegisterEventHandler);

	EYEX_DLL_HOOK(CreateSnapshot);
	EYEX_DLL_HOOK(AddSnapshotWindowId);
	EYEX_DLL_HOOK(CreateSnapshotBounds);
	EYEX_DLL_HOOK(CreateInteractor);
	EYEX_DLL_HOOK(CreateInteractorBounds);
	EYEX_DLL_HOOK(CreateInteractorBehavior);
	EYEX_DLL_HOOK(SetInteractorDeleted);
	EYEX_DLL_HOOK(CommitSnapshotAsync);
	EYEX_DLL_HOOK(GetEventInteractorId);

	void OnDeviceStatusChanged(TX_CONSTHANDLE asyncData);

	void OnConnectionStateChanged(TX_CONNECTIONSTATE ConnectionState);

	TX_CONTEXTHANDLE Context;
	std::function<void(int)>* LaunchCalibrationToolCallback;
	bool bConnected;

	mutable FCriticalSection mutex;
	bool bInConfiguration;

	// We handles eye position only
	void HandleEvent(TX_CONSTHANDLE AsyncData);

	void SetupEyePositionInteractor(bool makedDelete);

	const static std::string EyeXeyePositionInteractor;

	float XThresholod;
	float YThresholod;
	float ZThresholod;

	template <class F, class ...Params>
	class FGameThreadCallbackTask
	{
	public:
		FGameThreadCallbackTask(F&& func, Params&&... params)
		{
			mBoundFunc = std::bind(std::forward<F>(func), std::forward<Params>(params)...);
		}

		static const TCHAR* GetTaskName()
		{
			return TEXT("FGameThreadCallbackTask");
		}
		FORCEINLINE static TStatId GetStatId()
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FDynamicTextureCallbackTask, STATGROUP_TaskGraphTasks);
		}

		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::GameThread;
		}

		static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) {
			mBoundFunc();
		};

	private:
		std::function<void(void)> mBoundFunc;
	};
};

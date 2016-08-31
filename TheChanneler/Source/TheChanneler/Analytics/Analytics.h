#pragma once
#include "EyeXPlayerController.h"
#include "AnalyticsCommon.h"
#include "Analytics.generated.h"

class UAnalyticsModule;
class UAnalyticsSession;

UCLASS(Blueprintable, BlueprintType)
class THECHANNELER_API UAnalytics : public UObject
{
	GENERATED_BODY()
public:
	UAnalytics();

	/**
	* Tick Delegate
	* @param DeltaSeconds - seconds since lastest update
	*/
	bool Tick(float DeltaSeconds);
	void OnWorldChanged(UWorld* World, const UWorld::InitializationValues IVS);
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;

	/**
	* Get all analytics modules
	* @return the array of modules
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Analytic Modules"), Category = "Analytics")
	const TArray<UAnalyticsModule*>& GetAnalyticsModules() const { return Modules; };

	/**
	* Get the particular analytics module
	* @param objClass class
	* @return the module
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Analytics Module"), Category = "Analytics")
	UAnalyticsModule* GetAnalyticsModule(UClass* objClass) const;

	/** Start a new AnalyticsSession*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Session"), Category = "Analytics")
	void StartSession(const FString & userId);

	/** Start the AnalyticsSession*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Stop Session"), Category = "Analytics")
	void StopSession();

	/** Activate the particular analytics module
	* @param objClass class
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Activate Module"), Category = "Analytics")
	void ActivateModule(UClass* objClass);

	/** Deactivate the particular analytics module
	* @param objClass class
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Deactivate Module"), Category = "Analytics")
	void DeactivateModule(UClass* objClass);

	/**
	* Deactivate all modules
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Deactivate all modules"), Category = "Analytics")
	void DeactivateAllModules();

	/**
	* Activate all modules
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Activate all modules"), Category = "Analytics")
	void ActivateAllModules();

private:

	void InitAll();
	void CleanUpAll();

	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;

	/** Delegate for callbacks to Level changed */
	FDelegateHandle WorldCreatedDelegateHandle;

	UPROPERTY()
	TArray<UAnalyticsModule*> Modules;

	UPROPERTY()
	UAnalyticsSession* AnalyticsSession;

	mutable FCriticalSection Mutex;

	UWorld* AnalyticsWorld;
	ULevel* CurrentLevel;

	/** The static function to catch the first event before any UAnalytics was created*/
	static void OnWorldCreated(UWorld* World, const UWorld::InitializationValues IVS);
	class FStaticInitializer {
	public:
		FStaticInitializer() {
			OnWorldCreatedDelegate = FWorldDelegates::FWorldInitializationEvent::FDelegate::CreateStatic(&UAnalytics::OnWorldCreated);
			OnWorldCreatedDelegateHandle = FWorldDelegates::OnPostWorldInitialization.Add(OnWorldCreatedDelegate);
		}

		~FStaticInitializer() {
			FWorldDelegates::OnPostWorldInitialization.Remove(OnWorldCreatedDelegateHandle);
		}

		FWorldDelegates::FWorldInitializationEvent::FDelegate OnWorldCreatedDelegate;
		FDelegateHandle OnWorldCreatedDelegateHandle;
		UWorld* AnalyticsWorld;
		ULevel* CurrentLevel;
	};
	static FStaticInitializer Initializer;
};
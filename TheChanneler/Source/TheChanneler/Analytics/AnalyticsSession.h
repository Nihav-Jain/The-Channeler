#pragma once
#include "AnalyticsCommon.h"
#include "EyeXEx/EyeXPluginEx.h"
#include "AnalyticsSession.generated.h"

/**
* This class provides the information of a logging session
* TODO: This class only supports JSON serialization now.
* It might need to support an interface for different formats serialization in the future
*/
UCLASS(Blueprintable, BlueprintType)
class THECHANNELER_API UAnalyticsSession : public UObject
{
	GENERATED_BODY()

public:
	UAnalyticsSession();
	virtual ~UAnalyticsSession();

	virtual void PostInitProperties() override;

	/**
	* Update function for each Tick
	* @param DeltaSeconds - time in seconds since last Tick call
	*/
	void Update(float DeltaSeconds);

	/**
	* Get the common root path
	* @return the path
	*/
	const FString & GetRootPath() const {
		FScopeLock f(&Mutex); return RootPath;
	};;

	/**
	* Write the log Name/Value into the common log
	* @param section - the section name, use path format to do the nested section ex /sectionname1/sectionname2
	* @param name - the name of the pair
	* @param value - the value
	* @return true if succeed, or failed if the name exists and overwrite is not specified
	*/
	bool LogNameValueToSection(const FString & section, const FString & name, const FString & value, bool overwrite);

	/**
	* Write the log Name/Value into the common log
	* @param name - the name of the pair
	* @param value - the value
	* @return true if succeed, or failed if the name exists and overwrite is not specified
	*/
	bool LogNameValue(const FString & name, const FString & value, bool overwrite);

	/** Start the session */
	void Start(const FString & userId);
	/** Stop the session */
	void Stop();

	/** Pause the session */
	void Pause() { FScopeLock f(&Mutex); bIsPaused = true; };
	/** Resume the session */
	void Resume() { FScopeLock f(&Mutex); bIsPaused = false; };

	/**
	* On level switch event
	* @param world the world
	* @param level the level
	* @param name the level name
	*/
	void OnLevelChanged(UWorld * world, ULevel * level, const FString & name);

	/**
	* Get the level name
	* @return the name
	*/
	const FString & GetLevelName();

private:

	bool bIsPaused;
	bool bDirty; // TODO: Need to support streaming JSON into disk
	void WriteToLog();

	/** The JSON data object */
	TSharedRef<FJsonObject> Data;

	FDateTime SessionTime;
	FTimespan Duration;

	FString UserId;
	FGuid SessionId;
	FString RootPath;

	FDateTime LevelTime;
	FTimespan LevelDuration;
	FString LeveName;

	UEyeXPluginEx * EyeXEx;

	mutable FCriticalSection Mutex;
};
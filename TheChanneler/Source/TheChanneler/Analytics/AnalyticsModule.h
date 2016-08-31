#pragma once
#include "AnalyticsCommon.h"
#include "AnalyticsModule.generated.h"

class UAnalyticsSession;

/**
* The basic AnalyticsModule plugin class, a new plugin for analytics system should derive from this class
*/
UCLASS(Blueprintable, BlueprintType)
class THECHANNELER_API UAnalyticsModule : public UObject
{
	GENERATED_BODY()

	friend class UAnalytics;
public:
	virtual ~UAnalyticsModule() = default;

	/**
	* Init this module, perform initialization
	*/
	virtual void Init() {};

	/**
	* CleanUp this module, perform clean up work
	*/
	virtual void CleanUp() {};

	/**
	* Activate this module
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Activate the module"), Category = "Analytics")
	virtual void Activate() { bIsEnabled = true; };

	/**
	* Deactivate this module
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Deactive the module"), Category = "Analytics")
	virtual void Deactivate() { bIsEnabled = false; };

	/**
	* On level switch event
	* @param world the world
	* @param level the level
	* @param name the level name
	*/
	virtual void OnLevelChanged(UWorld * world, ULevel * level, const FString & name) {};

	/**
	* Update function for each Tick
	* @param DeltaSeconds - time in seconds since last Tick call
	*/
	virtual void Update(float DeltaSeconds) {};

	/**
	* Write the log Name/Value into the common log
	* @param section - the section name, use path format to do the nested section ex /sectionname1/sectionname2
	* @param name - the name of the pair
	* @param value - the value
	* @return true if succeed, or failed if the name exists and overwrite is not specified
	*/
	bool LogNameValueToSection(const FString & section, const FString & name, const FString & value, bool overwrite = false);

	/**
	* Write the log Name/Value into the common log
	* @param name the name of the pair
	* @param value the value
	* @return true if succeed, or failed if the name exists and overwrite is not specified
	*/
	bool LogNameValue(const FString & name, const FString & value, bool overwrite = false);

	/**
	* Get the module dependency path
	* @return the path
	*/
	const FString & GetModuleDependencyPath() const { return DependencyPath;};

	/**
	* Get the module log path
	* @return the path
	*/
	const FString & GetModuleLogPath() const { return LogPath;};

	/**
	* Get the module root path
	* @return the path
	*/
	const FString & GetModuleRootPath() const { return RootPath; };

	/**
	* Get the session root path
	* @return the path
	*/
	const FString & GetSessionRootPath() const;

	/**
	* Get the level name
	* @return the name
	*/
	const FString & GetLevelName();

protected:
	/**
	* Add any external file dependency for packaging
	* This function will not copy the file immediately into the folder, it only add to it the internal list
	* It waits after Init() is called, then copy the all files at once
	* @param absPath - the absolute path of the file
	*/
	void AddExternalDependency(const FString& absPath);

	/**
	* Exec a shell command, the external file dependency
	* the working directory, default to the module dependency path
	* This currently support *.py call, *.exe call, or shell open knowing file types
	* The executable or files need to be packaged with AddExternalDependency() first
	* Example:
	* 	ShellExec(TEXT("test.py 3"));
	*	ShellExec(TEXT("AutoScreenshot.png"));
	* Note: This is a non-blocking call
	* @param cmd - the cmd
	*/
	void ShellExec(const FString& cmd) const;

	UAnalyticsModule();
	bool bIsEnabled;

	/** Change to false to prevent the instace to be created*/
	bool bShouldBeInitiated;

private:
	void InitModule();

	void PackageExternalDependency();
	
	void SetAnalyticsSession(UAnalyticsSession* session) { AnalyticsSession = session; };

	UAnalyticsSession* AnalyticsSession;
	TArray<FString> ExternalDependencies;

	FString RootPath;
	FString LogPath;
	FString DependencyPath;
};
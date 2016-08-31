// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Analytics/AnalyticsModule.h"
#include "AnalyticsModule.h"
#include "BlinkWinkLogger.generated.h"

/**
 * Logs the status of both eyes every frame along with when the user blinked
 */
UCLASS()
class THECHANNELER_API UBlinkWinkLogger : public UAnalyticsModule
{
	GENERATED_BODY()
	
public:
	UBlinkWinkLogger();

	/**
	* Init this module, perform initialization
	*/
	virtual void Init() override;

	/**
	* Update function for each Tick
	* @param DeltaSeconds - time in seconds since last Tick call
	*/
	virtual void Update(float DeltaSeconds) override;

	/**
	* On level switch event
	* @param world the world
	* @param level the level
	* @param name the level name
	*/
	virtual void OnLevelChanged(UWorld* world, ULevel* level, const FString& name) override;

	/**
	* Activate the module from a specific puzzle.
	* @param puzzleName the name of the puzzle
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ActivateFromPuzzle"), Category = "Analytics")
	void StartLoggerFromPuzzle(const FString& puzzleName);

	/**
	* Activate this module
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Activate the module"), Category = "Analytics")
	virtual void Activate();

	/**
	* Deactivate this module
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Deactive the module"), Category = "Analytics")
	virtual void Deactivate();

	UPROPERTY(BlueprintReadWrite, Category="Analytics | BlinkWink", meta = (ClampMin = "1"))
	int32 MaxBufferBytes;

private:

	UENUM()
	enum class ELogSection : uint8
	{
		SECTION_NONE,
		SECTION_EYE_STATUS,
		SECTION_BLINK,
		SECTION_WINK,
		SECTION_BOTH_OPEN,
		SECTION_MAX
	};

	void MakeLogFileName();
	void CreateLogFileHeader();
	bool OpenFileHandle();
	void CloseFileHandle();
	void WriteBufferToFile();
	FBufferArchive* SaveSection(FBufferArchive& archive, ELogSection section);
	void WriteAndFlushArchive(FBufferArchive& archive);

	UFUNCTION()
	void UserBlinked();
	UFUNCTION()
	void LeftEyeClosed();
	UFUNCTION()
	void RightEyeClosed();
	UFUNCTION()
	void BothEyesOpen();

	FBufferArchive* mArchive;
	FBufferArchive* mEyeStatusLogData;
	FBufferArchive* mBlinkLogData;
	FBufferArchive* mWinkLogData;
	FBufferArchive* mBothOpenLogData;
	
	/** The name of the log file to be generated. */
	FString mLogFileName;

	/** The name of the puzzle that we are logging for. */
	FString PuzzleName;

	float mTotalDeltaTime;

	IFileHandle* mFileHandle;

	class AChannelerCharacter* mCharacter;
	UWorld* mWorld;

	int32 mTotalBytes;

	FScriptDelegate mBlinkDelegate;
	FScriptDelegate mLeftEyeClosedDelegate;
	FScriptDelegate mRightEyeClosedDelegate;
	FScriptDelegate mBothEyesOpenDelegate;

	bool mCacheCheckForClosedEye;
	bool bIsLevelChange;
};

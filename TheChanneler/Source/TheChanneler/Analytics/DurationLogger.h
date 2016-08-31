// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Analytics/AnalyticsModule.h"

#include "../Puzzles/Puzzle.h"

#include "DurationLogger.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, BlueprintType)
class THECHANNELER_API UDurationLogger : public UAnalyticsModule
{
	GENERATED_BODY()
	
	

private:
	struct FDurationInfo
	{
		float StartTime;
		float EndTime;

		FDurationInfo(float startTime, float endTime) : StartTime(startTime), EndTime(endTime) {}
	};

	TMap<FString, TArray<FDurationInfo>> PuzzleDurationTable;
	UWorld * WorldRef;
	FString LevelName;

	bool SaveResults();
	bool SaveStringTextToFile(const FString& SaveDirectory, const FString& JoyfulFileName, const FString& SaveText, bool AllowOverWriting);

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Event Time"), Category = "Duration Tracker")
	void StartEventTime(FString EventName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "End Event Time"), Category = "Duration Tracker")
	void EndEventTime(FString EventName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Nth Event Time"), Category = "Duration Tracker")
	void StartNthEventTime(FString EventName, int32 N);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "End Nth Event Time"), Category = "Duration Tracker")
	void EndNthEventTime(FString EventName, UPARAM(ref)int32& N);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Puzzle Time"), Category = "Duration Tracker")
	void StartPuzzleTime(APuzzle * Puzzle);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "End Puzzle Time"), Category = "Duration Tracker")
	void EndPuzzleTime(APuzzle * Puzzle);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Nth Puzzle Time"), Category = "Duration Tracker")
	void StartNthPuzzleTime(APuzzle * Puzzle, int32 N);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "End Nth Puzzle Time"), Category = "Duration Tracker")
	void EndNthPuzzleTime(APuzzle * Puzzle, UPARAM(ref)int32& N);

	// Store Level Name and generate Folder names based on that
	virtual void OnLevelChanged(UWorld * world, ULevel * level, const FString & name) override;

	// Deactivate the Heat Map module, perform the clean up work
	virtual void Deactivate() override;

	// On Clean Up, move all the visual logger data into their respective folders
	virtual void CleanUp() override;
};

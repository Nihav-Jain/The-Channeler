// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "DurationLogger.h"

void UDurationLogger::StartEventTime(FString EventName)
{
	// Get Duration List for this 
	TArray<FDurationInfo>& durationList = PuzzleDurationTable.FindOrAdd(EventName);

	float_t realTime = WorldRef->RealTimeSeconds;
	/*
	int32_t startTime;
	float_t startPartialTime;
	UGameplayStatics::GetAccurateRealTime(nullptr, startTime, startPartialTime);
	*/
	durationList.Add(FDurationInfo(realTime, -1.0f));
}

void UDurationLogger::EndEventTime(FString EventName)
{
	if (TArray<FDurationInfo> * durationList = PuzzleDurationTable.Find(EventName))
	{
		FDurationInfo& info = durationList->Last();
		info.EndTime = WorldRef->RealTimeSeconds;
	}
}

void UDurationLogger::StartNthEventTime(FString EventName, int32 N)
{
	StartEventTime(FString::Printf(TEXT("%s_%d"), *EventName, N));
}

void UDurationLogger::EndNthEventTime(FString EventName, UPARAM(ref)int32& N)
{
	EndEventTime(FString::Printf(TEXT("%s_%d"),*EventName, N));
	N++;
}

void UDurationLogger::StartPuzzleTime(APuzzle * Puzzle)
{
	StartEventTime(Puzzle->GetName());
}

void UDurationLogger::EndPuzzleTime(APuzzle * Puzzle)
{
	EndEventTime(Puzzle->GetName());
}

void UDurationLogger::StartNthPuzzleTime(APuzzle * Puzzle, int32 N)
{
	StartNthEventTime(Puzzle->GetName(), N);
}

void UDurationLogger::EndNthPuzzleTime(APuzzle * Puzzle, UPARAM(ref)int32& N)
{
	EndNthEventTime(Puzzle->GetName(), N);
}


void UDurationLogger::OnLevelChanged(UWorld * world, ULevel * level, const FString & name)
{
	// Save Results up to now
	WorldRef = world;
	SaveResults();
	LevelName = name;
}

void UDurationLogger::Deactivate()
{
	SaveResults();
}

void UDurationLogger::CleanUp()
{
	SaveResults();
}

bool UDurationLogger::SaveResults()
{
	// Make sure there are results to be saved
	if (PuzzleDurationTable.Num() <= 0)
		return true;

	// Create the CSV File
	FString csvText;
	csvText.Append(TEXT("Event Name, Number Of Entries, Total Duration, Average Duration"));

	// Calculate the total duration in the data

	// [Column Titles] + Num x [Event Name Info Row] + [Space] + [Column Titles] + [Beginning Row] = 4 + Num Rows
	int32_t previousOffset = 4 + PuzzleDurationTable.Num();
	int32_t currentOffset;
	int32_t eventOffset = 2; // Start event display at row 2
	for (auto& pair : PuzzleDurationTable)
	{
		FString& eventName = pair.Key;
		int32_t eventListEntries = pair.Value.Num();
		if (eventListEntries > 0)
		{
			// Calculate offsets to sum up
			currentOffset = previousOffset + eventListEntries - 1;
			csvText.Append(FString::Printf(TEXT("\n%s,%d,=SUM(C%d:C%d)-SUM(B%d:B%d),=C%d/B%d")
				, *eventName, eventListEntries, previousOffset, currentOffset, previousOffset, currentOffset, eventOffset, eventOffset));

			// Update
			previousOffset = currentOffset + 1;
			eventOffset++;
		}
	}

	// Print [Space] [Column Titles]
	csvText.Append(TEXT("\n\nEvent Name, Start Time, EndTime"));

	// Store the individual duration
	for (auto& pair : PuzzleDurationTable)
	{
		FString& eventName = pair.Key;
		TArray<FDurationInfo>& eventList = pair.Value;

		// Store the individual duration of this event
		for (FDurationInfo& data : eventList)
		{
			// If event was not ended, then end it on this time
			if (data.EndTime < data.StartTime)
			{
				data.EndTime = WorldRef->RealTimeSeconds;
			}
			csvText.Append(FString::Printf(TEXT("\n%s,%.2f,%.2f"), *eventName, data.StartTime, data.EndTime));
		}
	}

	// Clear the HashMap
	PuzzleDurationTable.Empty();

	// Create FileName
	FString JoyfulFileName;
	JoyfulFileName.Append(LevelName);
	JoyfulFileName.Append(FDateTime::Now().ToString());
	JoyfulFileName.Append(TEXT(".csv"));

	// Determine If Saved
	return SaveStringTextToFile(GetModuleLogPath(), JoyfulFileName, csvText, true);
}

// Resource for Text File creation:
// https://answers.unrealengine.com/questions/27254/writing-to-text-files.html
// https://wiki.unrealengine.com/index.php?title=File_Management,_Create_Folders,_Delete_Files,_and_More&action=edit
bool UDurationLogger::SaveStringTextToFile(const FString& SaveDirectory, const FString& JoyfulFileName, const FString& SaveText, bool AllowOverWriting)
{
	//Does the Directory Exists?
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*SaveDirectory))
	{
		//create directory if it not exist
		bool result = FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SaveDirectory);

		if (!result)
		{
			LOG("VLOG: Error, Directory Not Created!");
			return false;
		}
	}

	//Get complete file path
	FString FileDirectory = FString::Printf(TEXT("%s/%s"), *SaveDirectory, *JoyfulFileName);

	//No over-writing?
	if (!AllowOverWriting)
	{
		//Check if file exists already
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*FileDirectory))
		{
			//no overwriting
			return false;
		}
	}

	return FFileHelper::SaveStringToFile(SaveText, *FileDirectory);
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../Plugins/TobiiEyeX/Source/TobiiEyeX/Public/IEyeXPlugin.h"
#include "Kernel/GameInstanceBase.h"
#include "CoordinateLogger.h"

const float UCoordinateLogger::DefaultPollRate = 0.25f;
const bool UCoordinateLogger::DefaultEnabledValue = false;
const uint16 UCoordinateLogger::NumberOfCoordinatePairs = 50;

UCoordinateLogger::UCoordinateLogger() :
	BufferOffset(0), PollRate(DefaultPollRate), TimeBetweenPolls(0.0f), TotalTime(0.0f),
	CurrentInputPosition(FVector2D::ZeroVector), PreviousInputPosition(FVector2D::ZeroVector)
{
	UAnalyticsModule::bIsEnabled = DefaultEnabledValue;
}

void UCoordinateLogger::Init()
{
	EyeX = &IEyeXPlugin::Get();

	Activate();
}

void UCoordinateLogger::Update(float DeltaSeconds)
{
	TotalTime += DeltaSeconds;
	TimeBetweenPolls += DeltaSeconds;
	if (TimeBetweenPolls >= PollRate)
	{
		CaptureState();
		TimeBetweenPolls = 0;
	}
}

void UCoordinateLogger::OnLevelChanged(UWorld* world, ULevel* level, const FString& name)
{
	Deactivate();
	Activate();
}

void UCoordinateLogger::StartLoggerFromPuzzle(const FString& puzzleName)
{
	Deactivate();
	PuzzleName = puzzleName;
	Activate();
}

void UCoordinateLogger::Activate()
{
	if (bIsEnabled)
		return;

	UAnalyticsModule::Activate();

	// Each pair is an int16 so we can simply multiply the pairs by a 32-bit integer
	ByteArray = reinterpret_cast<uint8*>(FMemory::Malloc(NumberOfCoordinatePairs * sizeof(int32)));

	// Initialize members
	BufferOffset = 0;
	PollRate = DefaultPollRate;
	TimeBetweenPolls = 0.0f;
	TotalTime = 0.0f;
	CurrentInputPosition = FVector2D::ZeroVector;
	PreviousInputPosition = FVector2D::ZeroVector;

	// Create the new file directory
	CreateLogFile();
}

void UCoordinateLogger::Deactivate()
{
	if (bIsEnabled)
	{
		// Write the remaining coordinates then free the buffer
		WriteBufferToFile();
		FMemory::Free(ByteArray);
	}

	bIsEnabled = false;
}

void UCoordinateLogger::CaptureState()
{
	// Update the time and position
	GetCurrentTime();
	UpdateInputPosition();

	// Delta encode and then truncate decimal places for 2D coordinates
	int16 PositionX = static_cast<int16>(CurrentInputPosition.X - PreviousInputPosition.X);
	int16 PositionY = static_cast<int16>(CurrentInputPosition.Y - PreviousInputPosition.Y);

	// Write X and Y coordinates to byte array
	// TODO: Consider refactoring - This probably shouldn't calculated each tick.
	// It may be better to store only once for each chunk and increment the pointer
	// This will also remove the need to the BufferOffset
	int16* IntByteArrayPointer = reinterpret_cast<int16*>(ByteArray);
	IntByteArrayPointer += BufferOffset;

	*IntByteArrayPointer = PositionX;
	++IntByteArrayPointer;

	*IntByteArrayPointer = PositionY;
	++IntByteArrayPointer;

	// TODO: Have someone double check logic
	// Why does this work? I don't understand why this isn't doubled...
	// We have two 16-bit integers (one X the other Y) yet we only increment the buffer offset
	// by the number of bytes in a 16-bit integer (using 32 bits leaves up with uninitialized memory
	// being written to the log)
	BufferOffset += sizeof(int16);

	if ((BufferOffset * 2) >= (NumberOfCoordinatePairs * sizeof(int32)))
	{
		WriteBufferToFile();
	}
}

void UCoordinateLogger::UpdateInputPosition()
{
	PreviousInputPosition = CurrentInputPosition;
	FVector2D input = GetInputPosition();
	CurrentInputPosition.Set(input.X, input.Y);
}

/* TODO: (Steven says...) Consider making the CoordinateLogger an abstract class now that it's being multi-purposed.
 * Let the EyeX input be in a separate class, just like how the PositionLogger is its own class (inheriting from this class).
 * Let the GetInputPosition be pure virtual and override it in the EyeXLogger to do exactly the code below.
 * (although idk how well pure virtual is supported in Unreal, and especially with the Analytics tool... all food for thought).
 */
FVector2D UCoordinateLogger::GetInputPosition()
{
	return EyeX->GetGazePoint().Value;
}

void UCoordinateLogger::CreateLogFile()
{
	const FString fileExt = ".bin";

	LogFileName = GetModuleLogPath() + PuzzleName + "_" + FDateTime::Now().ToString() + fileExt;

	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* Handle = File.OpenWrite(*LogFileName, true);
	if (!Handle)
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ERROR: CoordinateLogger failed to open file. Logging data will not be recorded!"));
	}
	else
	{
		// Binary data is written out to disk based on the platform that is writing it.
		// Since we are only supporting Windows there is no need to worry about an endianness flag - 
		// one byte is hardcoded to little-endian (0) but a check may be necessary to support big-endian (1)
		// in the future.
		//
		// Likewise with the poll rate, a configuration was previously supported for polling rates between 0 and 1 seconds.
		// It is currently hardcoded to poll every 0.25s (1). In the future we can support 0.00s (0), 0.25s (1), 0.50s (2), 0.75s(3), 1.00s (4)
		// but is currently unnecessary since designers seemed to favor 0.25s in all cases.

		// Write header information
		uint8 EndiannessFlag = 0;
		Handle->Write(&EndiannessFlag, sizeof(EndiannessFlag));

		uint8 PollRateFlag = 1;
		Handle->Write(&PollRateFlag, sizeof(PollRateFlag));

		// Write the first coordinate pair - the first pair is the raw coordinate
		// Each pair after will be the delta
		UpdateInputPosition();
		int16* IntByteArrayPointer = reinterpret_cast<int16*>(ByteArray);
		*IntByteArrayPointer = static_cast<int16>(CurrentInputPosition.X);
		++IntByteArrayPointer;
		*IntByteArrayPointer = static_cast<int16>(CurrentInputPosition.Y);
		Handle->Write(ByteArray, sizeof(int32));

		// Close the file
		delete Handle;
	}
}

void UCoordinateLogger::WriteBufferToFile()
{
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* Handle = File.OpenWrite(*LogFileName, true);
	if (Handle)
	{
		Handle->Write(ByteArray, BufferOffset * 2);
		BufferOffset = 0;

		// Close the file
		delete Handle;
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "AnalyticsModule.h"
#include "Archive.h"
#include "FileManager.h"
#include "CoordinateLogger.generated.h"

/**
 * Logs the x and y coordinates for an input device. Both coordinates and
 * timestamps are written and all data may be written to an external file.
 */
UCLASS()
class THECHANNELER_API UCoordinateLogger : public UAnalyticsModule
{
	GENERATED_BODY()
	
public:	
	/**
	 * Constructor.
	 */
	UCoordinateLogger();

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
	* Captures the current state of the game. In particular, this class will handle
	* capturing the position of the input device and the current time in the world.
	*/
	void CaptureState();

	/**
	* Updates the current and previous positions of the input device as a 2D vector.
	*/
	void UpdateInputPosition();


	/**
	* Gets the current positions of the input device as a 2D vector.
	* Override this method to allow different input sources to utilize the 2D heatmap system.
	* e.g. PositionLogger overrides this and returns the player's (x,y) position in the world.
	*/
	virtual FVector2D GetInputPosition();


	/**
	* Create the name of the file based on a prefix, timestamp, and extension.
	*/
	void CreateLogFile();

	/**
	 * Activate the module from a specific puzzle.
	 * @param puzzleName the name of the puzzle
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ActivateFromPuzzle"), Category = "Analytics")
	void StartLoggerFromPuzzle(const FString& puzzleName);

	/**
	 * Activate this module
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Activate - Depreciated"), Category = "Analytics")
	virtual void Activate();

	/**
	 * Deactivate this module
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Deactivate"), Category = "Analytics")
	virtual void Deactivate();

private:
	/** Default values for variables within the Unreal Editor */
	static const bool DefaultEnabledValue;
	static const float DefaultPollRate;
	static const uint16 NumberOfCoordinatePairs;

	/** Eyex used for getting the Gaze data. */
	class IEyeXPlugin* EyeX;

	/** A byte array used to hold information about coordinates */
	uint8* ByteArray;

	/** The memory offset to the current location to write to in the byte array */
	uint32 BufferOffset;

	/** The name of the log file to be generated. */
	FString LogFileName;

	/** The name of the puzzle that we are logging for. */
	FString PuzzleName;

	/** The rate at which data is gathered. */
	float TimeBetweenPolls;

	/** Total time the logger has been running for. This should be equivalent to the total game time. */
	float TotalTime;

	/** The rate at which the capture state gets called in the CoordinateLogger. */
	float PollRate;

	/** The current position of the input device. */
	FVector2D CurrentInputPosition;

	/** The position of the input device from the last update. */
	FVector2D PreviousInputPosition;

	/** Writes (by appending) a binary buffer out to a file */
	void WriteBufferToFile();
};

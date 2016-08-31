// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Analytics/CoordinateLogger.h"
#include "PositionLogger.generated.h"

/**
 * 
 */
UCLASS()
class THECHANNELER_API UPositionLogger : public UCoordinateLogger
{
	GENERATED_BODY()
private:
	ACharacter* MainPlayer;
	UWorld* MainWorld;
	int NumberOfFindingPlayerAttempts;
public:
	/**
	* Constructor.
	*/
	UPositionLogger();

	/**
	* Gets the current player in this world. Returns false if player not found.
	*/
	bool FindPlayerCharacter();

	/**
	* Gets the current player's position as a 2D vector.
	*/
	FVector2D GetInputPosition() override;

	/**
	* On level switch event. Used to retrieve the main player from the current world / level.
	* @param world the world
	* @param level the level
	* @param name the level name
	*/
	virtual void OnLevelChanged(UWorld* world, ULevel* level, const FString& name) override;
};

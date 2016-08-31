// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "PositionLogger.h"

UPositionLogger::UPositionLogger() : UCoordinateLogger(), MainPlayer(nullptr) {}

bool UPositionLogger::FindPlayerCharacter()
{
	// Update Number of Attempts
	if (NumberOfFindingPlayerAttempts <= 0)
	{
		return false;
	}
	else
	{
		--NumberOfFindingPlayerAttempts;
	}

	// Set MainPlayer to null
	MainPlayer = nullptr;

	// Get the MainPlayer from this new world
	if (MainWorld == nullptr)
	{
		LOG("POSLOG: World is null! Cannot retrieve Main Player.");
	}
	else
	{
		APlayerController* playerController = MainWorld->GetFirstPlayerController();

		if (playerController == nullptr)
		{
			LOG("POSLOG: Player Controller is Null! Cannot retrieve Main Player.");
		}
		else
		{
			MainPlayer = playerController->GetCharacter();

			// If MainPlayer is still null, just return bad values
			if (MainPlayer == nullptr)
			{
				LOG("POSLOG: PlayerController has no Player Character? IMPOSSIBRUU!");
			}
			else
			{	// Main Player Found, return successful
				LOG("POSLOG: PlayerCharacter is Found.");
				return true;
			}
		}
	}
	return false;
}

FVector2D UPositionLogger::GetInputPosition()
{
	if (MainPlayer != nullptr || FindPlayerCharacter())
	{
		// Get Player Position
		FVector position = MainPlayer->GetActorLocation();
		return FVector2D(position.X, position.Y);
	}
	else
	{
		// No Player exists, return bad values...
		return FVector2D(-1.0f, -1.0f);
	}
}


void UPositionLogger::OnLevelChanged(UWorld* world, ULevel* level, const FString& name)
{
	MainWorld = world;
	NumberOfFindingPlayerAttempts = 6;
	FindPlayerCharacter();
	UCoordinateLogger::OnLevelChanged(world, level, name);
}

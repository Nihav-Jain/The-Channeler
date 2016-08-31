// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "ChannelerGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestUpdated, FString, NewQuest);


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class THECHANNELER_API AChannelerGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AChannelerGameState();

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FQuestUpdated QuestUpdated;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	const FString& GetPlayerQuest();

	void UpdateQuest(const FString& newQuest);

private:
	FString mPlayerQuest;
};

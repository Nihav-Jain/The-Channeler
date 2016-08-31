// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerGameState.h"

AChannelerGameState::AChannelerGameState() :
	mPlayerQuest("Look here!!"), QuestUpdated()
{}

const FString& AChannelerGameState::GetPlayerQuest()
{
	return mPlayerQuest;
}

void AChannelerGameState::UpdateQuest(const FString& newQuest)
{
	mPlayerQuest = newQuest;

	if (QuestUpdated.IsBound())
		QuestUpdated.Broadcast(mPlayerQuest);
}

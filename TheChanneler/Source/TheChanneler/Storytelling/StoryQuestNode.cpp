// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../Kernel/ChannelerGameState.h"
#include "StoryQuestNode.h"

UStoryQuestNode::UStoryQuestNode() :
	mQuest("Stare at this")
{
}

void UStoryQuestNode::Initialize(const FStoryDataTable& initRow)
{
	mQuest = initRow.Speaker;
	Super::Initialize(initRow);
}

void UStoryQuestNode::Activate()
{
	if (mWorld == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UStoryQuestNode::Activate() %s - World is null"), *mNodeName);
		return;
	}
	AChannelerGameState* gameState = mWorld->GetGameState<AChannelerGameState>();
	if (gameState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UStoryQuestNode::Activate() %s - GameState is null"), *mNodeName);
		return;
	}
	gameState->UpdateQuest(mQuest);
	Manager->GotoNextNode();
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../Storytelling/StoryManager.h"
#include "ChannelerCheatManager.h"

UChannelerCheatManager::UChannelerCheatManager() :
	mStoryManager(nullptr)
{}

void UChannelerCheatManager::InitCheatManager()
{
	Super::InitCheatManager();
	// Any initialize stuff You need.
	UE_LOG(LogTemp, Warning, TEXT("CheatManager Init"));
}

void UChannelerCheatManager::SetStoryManager(UStoryManager& storyManager)
{
	mStoryManager = &storyManager;
}

void UChannelerCheatManager::SkipCurrentStoryNode()
{
	UE_LOG(LogTemp, Warning, TEXT("UChannelerCheatManager::SkipStoryNode()"));
	if (mStoryManager != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("skipping node"));
		mStoryManager->SkipNode();
	}
}
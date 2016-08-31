// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerCheatManager.h"
#include "../Storytelling/StoryNode.h"
#include "../Storytelling/StoryDialogueFreezeCamNode.h"
#include "../Utils/ChannelerUtils.h"
#include "ChannelerEyeXPlayerController.h"

AChannelerEyeXPlayerController::AChannelerEyeXPlayerController() :
	mCheatManager(nullptr)
{
	UChannelerUtils::SetChannelerPlayerController(this);
}

void AChannelerEyeXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (mCheatManager == nullptr)
	{
		mCheatManager = NewObject<UChannelerCheatManager>(this, UChannelerCheatManager::StaticClass());
		mCheatManager->InitCheatManager();
	}

}

UChannelerCheatManager& AChannelerEyeXPlayerController::CheatManager()
{
	check(mCheatManager != nullptr);
	return *mCheatManager;
}

void AChannelerEyeXPlayerController::SetStoryManager(UStoryManager& storyManager)
{
	mStoryManager = &storyManager;
}

void AChannelerEyeXPlayerController::SkipCurrentStoryNode()
{
	UE_LOG(LogTemp, Warning, TEXT("UChannelerCheatManager::SkipStoryNode()"));
	if (mStoryManager != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("skipping node"));
		mStoryManager->SkipNode();
	}
}

void AChannelerEyeXPlayerController::TransitionToDialogueCam()
{
	if (mStoryManager == nullptr)
		return;

	UStoryNode* currentNode = mStoryManager->GetCurrentNode();
	if (currentNode != nullptr)
	{
		UStoryDialogueFreezeCamNode* node = Cast<UStoryDialogueFreezeCamNode>(currentNode);
		if (node != nullptr)
		{
			node->mFreezeCam->TransitionToDialogueCam();
		}
	}
}

void AChannelerEyeXPlayerController::TransferControlToPlayer()
{
	if (mStoryManager == nullptr)
		return;

	UStoryNode* currentNode = mStoryManager->GetCurrentNode();
	if (currentNode != nullptr)
	{
		UStoryDialogueFreezeCamNode* node = Cast<UStoryDialogueFreezeCamNode>(currentNode);
		if (node != nullptr)
		{
			node->mFreezeCam->EndInteraction();
		}
	}
}

void AChannelerEyeXPlayerController::JumpToStoryNode(FString nodeName)
{
	if (mStoryManager == nullptr)
		return;

	UStoryNode* nodeToJump = mStoryManager->GetNodeByName(nodeName);
	if (nodeToJump != nullptr)
		mStoryManager->SetCurrentNode(nodeToJump, true);
}
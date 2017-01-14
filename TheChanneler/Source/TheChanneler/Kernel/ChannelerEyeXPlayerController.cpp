// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerCheatManager.h"
#include "../Storytelling/StoryNode.h"
#include "../Storytelling/StoryDialogueFreezeCamNode.h"
#include "../Utils/ChannelerUtils.h"
#include "GameInstanceBase.h"
#include "ChannelerEyeXPlayerController.h"
#include "../Input/InputDeviceManager.h"

AChannelerEyeXPlayerController::AChannelerEyeXPlayerController()
	: mCheatManager(nullptr)
	, InputDeviceManager(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	UChannelerUtils::SetChannelerPlayerController(this);
	InputDeviceManager = NewObject<UInputDeviceManager>(this, UInputDeviceManager::StaticClass(), TEXT("InputDeviceManager"));
	UChannelerUtils::SetInputDeviceManager(InputDeviceManager);
}

void AChannelerEyeXPlayerController::BeginPlay()
{
	if (InputDeviceManager == nullptr)
	{
		InputDeviceManager = NewObject<UInputDeviceManager>(this, UInputDeviceManager::StaticClass(), TEXT("InputDeviceManager"));
		UChannelerUtils::SetInputDeviceManager(InputDeviceManager);
	}

	Super::BeginPlay();

	if (mCheatManager == nullptr)
	{
		mCheatManager = NewObject<UChannelerCheatManager>(this, UChannelerCheatManager::StaticClass());
		mCheatManager->InitCheatManager();
	}

	SetActorTickEnabled(true);
	EnableInput(this);
}

void AChannelerEyeXPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	InputDeviceManager->Tick();
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

void AChannelerEyeXPlayerController::PrintScreenResolution()
{
	UGameUserSettings* gameSettings = GEngine->GetGameUserSettings();
	if (gameSettings != nullptr)
	{
		FIntPoint currentScreenRes = gameSettings->GetScreenResolution();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("%d x %d"), currentScreenRes.X, currentScreenRes.Y));
		UE_LOG(LogTemp, Warning, TEXT("%d x %d"), currentScreenRes.X, currentScreenRes.Y);
	}
}


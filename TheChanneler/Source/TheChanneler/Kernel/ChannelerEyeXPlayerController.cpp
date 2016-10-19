// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerCheatManager.h"
#include "../Storytelling/StoryNode.h"
#include "../Storytelling/StoryDialogueFreezeCamNode.h"
#include "../Utils/ChannelerUtils.h"
#include "ChannelerEyeXPlayerController.h"

AChannelerEyeXPlayerController::AChannelerEyeXPlayerController() :
	mCheatManager(nullptr), mLastKnownInputDevice(EInputDevices::ID_KBM)
{
	PrimaryActorTick.bCanEverTick = true;

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

	SetActorTickEnabled(true);
	EnableInput(this);
}

void AChannelerEyeXPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (WasGamepadInputJustDetected())
	{
		ClientMessage("Gamepad was activated");
		mLastKnownInputDevice = EInputDevices::ID_Gamepad;
	}
	else if (WasKMBInputJustDetected())
	{
		ClientMessage("Keyboard/Mouse was activated");
		mLastKnownInputDevice = EInputDevices::ID_KBM;
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

EInputDevices AChannelerEyeXPlayerController::GetLastKnownInputDevice() const
{
	return mLastKnownInputDevice;
}

bool AChannelerEyeXPlayerController::WasKMBInputJustDetected() const
{
	return
		WasInputKeyJustPressed(EKeys::MouseX) ||
		WasInputKeyJustPressed(EKeys::MouseY) ||
		WasInputKeyJustPressed(EKeys::LeftMouseButton) ||
		WasInputKeyJustPressed(EKeys::RightMouseButton) ||
		WasInputKeyJustPressed(EKeys::W) ||
		WasInputKeyJustPressed(EKeys::A) ||
		WasInputKeyJustPressed(EKeys::A) ||
		WasInputKeyJustPressed(EKeys::D) ||
		WasInputKeyJustPressed(EKeys::P) ||
		WasInputKeyJustPressed(EKeys::Tab) ||
		WasInputKeyJustPressed(EKeys::Enter) ||
		WasInputKeyJustPressed(EKeys::Escape) ||
		WasInputKeyJustPressed(EKeys::SpaceBar);
}

bool AChannelerEyeXPlayerController::WasGamepadInputJustDetected() const
{
	return
		WasInputKeyJustPressed(EKeys::Gamepad_LeftX) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftY) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightX) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightY) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftTriggerAxis) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightTriggerAxis) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftThumbstick) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightThumbstick) ||
		WasInputKeyJustPressed(EKeys::Gamepad_Special_Left) ||
		WasInputKeyJustPressed(EKeys::Gamepad_Special_Right) ||
		WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom) ||
		WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Right) ||
		WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Left) ||
		WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Top) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftShoulder) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightShoulder) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftTrigger) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightTrigger) ||
		WasInputKeyJustPressed(EKeys::Gamepad_DPad_Up) ||
		WasInputKeyJustPressed(EKeys::Gamepad_DPad_Down) ||
		WasInputKeyJustPressed(EKeys::Gamepad_DPad_Right) ||
		WasInputKeyJustPressed(EKeys::Gamepad_DPad_Left) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftStick_Up) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftStick_Down) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftStick_Right) ||
		WasInputKeyJustPressed(EKeys::Gamepad_LeftStick_Left) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightStick_Up) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightStick_Down) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightStick_Right) ||
		WasInputKeyJustPressed(EKeys::Gamepad_RightStick_Left);
}

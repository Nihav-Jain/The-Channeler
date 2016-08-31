// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../Camera/DialogueFreezeCam.h"
#include "StoryManager.h"
#include "StoryDialogueFreezeCamNode.h"

TMap<FString, ADialogueFreezeCam*> UStoryDialogueFreezeCamNode::sCamMap;

UStoryDialogueFreezeCamNode::UStoryDialogueFreezeCamNode() :
	mFreezeCam(nullptr), mAction("BackToPlayer"), mCameraTransitionDelegate(),
	AboutToStartPuzzle()
{
	bCanMoveToNext = false;
	mCameraTransitionDelegate.BindUFunction(this, "CameraTransitionComplete");
}

UStoryDialogueFreezeCamNode::~UStoryDialogueFreezeCamNode()
{
	if (sCamMap.Num() != 0)
		sCamMap.Empty();
}

void UStoryDialogueFreezeCamNode::Activate()
{
	if (mFreezeCam == nullptr)
		return;

	UE_LOG(LogTemp, Warning, TEXT("UStoryDialogueFreezeCamNode::Activate() : %s, %s"), *(mFreezeCam->CameraName), *mAction);

	mFreezeCam->EnableFreezeCam();
	
	// TODO: replace string comparison with enums
	if (mAction == "ToDialogueCam")
	{
		if (!mFreezeCam->TransitionOnOverlap)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s transitioning to dialogue freeze cam automatically"), *(mFreezeCam->CameraName));
			mFreezeCam->TransitionToDialogueCam();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s cannot transition to dialogue freeze cam automatically"), *(mFreezeCam->CameraName));
		}
	}
	else if (mAction == "ToPuzzleCam")
	{
		mFreezeCam->TransitionToPuzzleCam();
	}
	else if (mAction == "BackToPlayer")
	{
		mFreezeCam->DisableFreezeCam();
		mFreezeCam->EndInteraction();
	}
}

void UStoryDialogueFreezeCamNode::Initialize(const FStoryDataTable& initRow)
{
	PopulateDialogueFreezeCams();
	if (sCamMap.Contains(initRow.Speaker))
	{
		mFreezeCam = sCamMap[initRow.Speaker];
		mAction = initRow.Dialogue;

		mFreezeCam->DisableFreezeCam();
		mFreezeCam->CameraTransitionComplete.AddUnique(mCameraTransitionDelegate);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("UStoryDialogueFreezeCamNode: DialogueFreezeCam %s does not exist"), *initRow.Speaker);
	Super::Initialize(initRow);
}

void UStoryDialogueFreezeCamNode::PopulateDialogueFreezeCams()
{
	if (sCamMap.Num() == 0)
	{
		for (TActorIterator<ADialogueFreezeCam> ActorItr(mWorld); ActorItr; ++ActorItr)
		{
			ADialogueFreezeCam* actor = *ActorItr;
			if (actor == nullptr)
				continue;

			if (sCamMap.Contains(actor->CameraName))
			{
				UE_LOG(LogTemp, Error, TEXT("UStoryDialogueFreezeCamNode: DialogueFreezeCam %s already exists. Previous value will be overriden."), *(actor->CameraName));
				sCamMap[actor->CameraName] = actor;
			}
			else
				sCamMap.Add(actor->CameraName, actor);
		}
	}
}

void UStoryDialogueFreezeCamNode::CameraTransitionComplete(EDialogueCamState oldState, EDialogueCamState newState)
{
	if (Manager->GetCurrentNode() != this)
		return;

	UE_LOG(LogTemp, Warning, TEXT("UStoryDialogueFreezeCamNode::CameraTransitionComplete %d %d"), (int32)oldState, (int32)newState);
	mFreezeCam->DisableFreezeCam();

	// go from any cam to puzzle cam
	if (newState == EDialogueCamState::PUZZLE_CAM_ACTIVE)
	{
		if (AboutToStartPuzzle.IsBound())
			AboutToStartPuzzle.Broadcast();
		mFreezeCam->StartPuzzle();
	}
	// go from idle to dialogue cam; go from any cam back to player
	else if (oldState == EDialogueCamState::IDLE || newState == EDialogueCamState::IDLE)
	{
		UpdateCanMoveToNext(true);
		Manager->GotoNextNode();
	}
	// go from puzzle cam to dialogue cam
	else if (oldState == EDialogueCamState::PUZZLE_CAM_ACTIVE && newState == EDialogueCamState::DIALOGUE_CAM_ACTIVE)
	{
		UpdateCanMoveToNext(true);
		Manager->GotoNextNode();
	}
}
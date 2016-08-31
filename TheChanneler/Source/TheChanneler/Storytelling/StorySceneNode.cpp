// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "StoryManager.h"
#include "StorySceneNode.h"

UStorySceneNode::UStorySceneNode() :
	mAutoComplete(false)
{
	bCanMoveToNext = false;
}

void UStorySceneNode::Activate()
{
	Super::Activate();
	if (StartScene.IsBound())
		StartScene.Broadcast();

	if (mAutoComplete)
		SetSceneComplete();
}

void UStorySceneNode::Initialize(const FStoryDataTable& initRow)
{
	mAutoComplete = (initRow.Speaker == "true") ? true : false;
}

void UStorySceneNode::SetSceneComplete()
{
	UpdateCanMoveToNext(true);
	if (Manager != nullptr)
	{
		if (Manager->GetCurrentNode() == this)
		{
			Manager->GotoNextNode();
		}
	}
}


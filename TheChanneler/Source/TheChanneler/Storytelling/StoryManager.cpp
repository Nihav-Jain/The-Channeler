// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "StoryNode.h"
#include "../Utils/ChannelerUtils.h"
#include "StoryManager.h"

UStoryManager::UStoryManager() :
	mRootStoryNode(nullptr), mCurrentNode(nullptr),
	bIsActive(false), mNodeMap()
{
	UChannelerUtils::SetStoryManager(this);
}

UStoryNode* UStoryManager::AddNextNode(UClass* className, const FString& nodeName)
{
	UStoryNode* node = NewObject<UStoryNode>(this, className);

	if (mCurrentNode == nullptr)
	{
		mRootStoryNode = node;
		mCurrentNode = node;
	}
	else
	{
		mCurrentNode->AddNextNode(node);
		mCurrentNode = node;
	}
	node->SetWorld(*GetWorld());
	node->Manager = this;

	if (mNodeMap.Contains(nodeName))
	{
		UE_LOG(LogTemp, Error, TEXT("Duplicate node name %s, previous value will be overriden"), *nodeName);
		mNodeMap[nodeName] = node;
	}
	else
	{
		mNodeMap.Add(nodeName, node);
	}

	return mCurrentNode;
}

void UStoryManager::GotoNextNode()
{
	if (mCurrentNode != nullptr)
	{
		bool canGotoNext = mCurrentNode->Deactivate();
		if (canGotoNext)
		{
			mCurrentNode = &mCurrentNode->NextNode();			
			if (mCurrentNode != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Node %s Activated"), *(mCurrentNode->NodeName()));
				mCurrentNode->Activate();
			}
			else
			{
				if (StoryEnded.IsBound())
					StoryEnded.Broadcast();
			}
		}
	}
}

void UStoryManager::BeginStory()
{
	mCurrentNode = mRootStoryNode;

	if(mCurrentNode != nullptr)
		mCurrentNode->Activate();

	ActivateStoryManager();

	if (StoryBegun.IsBound())
		StoryBegun.Broadcast();
}

void UStoryManager::ActivateStoryManager()
{
	if (!bIsActive && StoryManagerActivated.IsBound())
	{
		StoryManagerActivated.Broadcast();
	}
	bIsActive = true;
}

void UStoryManager::DeactivateStoryManager()
{
	if (bIsActive && StoryManagerDeactivated.IsBound())
	{
		StoryManagerDeactivated.Broadcast();
	}
	bIsActive = false;
}

void UStoryManager::SkipNode()
{
	GotoNextNode();
}

UStoryNode* UStoryManager::GetNodeByName(FString nodeName) const
{
	if (mNodeMap.Contains(nodeName))
		return mNodeMap[nodeName];
	UE_LOG(LogTemp, Error, TEXT(" UStoryManager::GetNodeByName() - Node name %s not found"), *nodeName);
	return nullptr;
}

UStoryNode* UStoryManager::GetCurrentNode() const
{
	return mCurrentNode;
}

void UStoryManager::SetCurrentNode(UStoryNode* node, bool activate)
{
	if (mCurrentNode != nullptr)
		mCurrentNode->Deactivate();
	mCurrentNode = node;
	if (activate && mCurrentNode != nullptr)
		mCurrentNode->Activate();
}
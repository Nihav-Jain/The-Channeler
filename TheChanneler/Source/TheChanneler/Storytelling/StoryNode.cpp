// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "StoryNode.h"


UStoryNode::UStoryNode() :
	mNextNode(nullptr), bRequiresTick(false), bCanMoveToNext(true), mWorld(nullptr),
	mNodeName(), Manager(nullptr)
{}

void UStoryNode::AddNextNode(UStoryNode* next)
{
	check(mNextNode == nullptr);
	mNextNode = next;
}

const UStoryNode& UStoryNode::NextNode() const
{
	check(mNextNode != nullptr);
	return *mNextNode;
}

UStoryNode& UStoryNode::NextNode()
{
	return *mNextNode;
}

bool UStoryNode::RequiresTick() const
{
	return bRequiresTick;
}

bool UStoryNode::CanMoveToNext() const
{
	return bCanMoveToNext;
}

void UStoryNode::UpdateCanMoveToNext(bool value)
{
	bCanMoveToNext = value;
}

void UStoryNode::Initialize(const FStoryDataTable& initRow)
{
	mNodeName = initRow.NodeName;
	Init(initRow);
}

void UStoryNode::SetWorld(UWorld& world)
{
	mWorld = &world;
}

const FString& UStoryNode::NodeName() const
{
	return mNodeName;
}
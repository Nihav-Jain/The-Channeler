// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "StoryManager.h"
#include "StoryJumpNode.h"

UStoryJumpNode::UStoryJumpNode() :
	bStartImmediately(true)
{}

void UStoryJumpNode::JumpToNode()
{
	check(Manager != nullptr);
	Manager->SetCurrentNode(this, bStartImmediately);
}

void UStoryJumpNode::Activate()
{
	check(Manager != nullptr);
	Manager->GotoNextNode();
}

void UStoryJumpNode::Initialize(const FStoryDataTable& initRow)
{
	bStartImmediately = (initRow.Speaker == "true") ? true : false;
	Super::Initialize(initRow);
}
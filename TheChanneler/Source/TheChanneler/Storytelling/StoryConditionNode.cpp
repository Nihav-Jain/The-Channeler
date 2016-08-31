// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "StoryConditionNode.h"

UStoryConditionNode::UStoryConditionNode()
{
	bRequiresTick = true;
	bCanMoveToNext = false;
}

void UStoryConditionNode::Initialize(const FStoryDataTable& initRow)
{
	ConditionName = initRow.Speaker;
}

void UStoryConditionNode::SetStoryCondition(bool value)
{
	UpdateCanMoveToNext(value);
}
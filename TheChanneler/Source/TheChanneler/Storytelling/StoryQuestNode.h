// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Storytelling/StoryNode.h"
#include "StoryQuestNode.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class THECHANNELER_API UStoryQuestNode : public UStoryNode
{
	GENERATED_BODY()
	
public:
	UStoryQuestNode();

	virtual void Initialize(const FStoryDataTable& initRow) override;
	virtual void Activate() override;

private:
	FString mQuest;
	
	
};

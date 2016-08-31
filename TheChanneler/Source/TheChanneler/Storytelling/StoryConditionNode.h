// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Storytelling/StoryNode.h"
#include "StoryDataTable.h"
#include "StoryConditionNode.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class THECHANNELER_API UStoryConditionNode : public UStoryNode
{
	GENERATED_BODY()
	
public:
	UStoryConditionNode();

	FString ConditionName;
	
	virtual void Initialize(const FStoryDataTable& initRow) override;

	UFUNCTION(BlueprintCallable, Category="Storytelling")
	void SetStoryCondition(bool value);
};

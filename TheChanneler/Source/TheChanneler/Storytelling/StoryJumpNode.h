// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Storytelling/StoryNode.h"
#include "StoryJumpNode.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class THECHANNELER_API UStoryJumpNode : public UStoryNode
{
	GENERATED_BODY()
	
public:
	UStoryJumpNode();
	
	UFUNCTION(BlueprintCallable, Category="Storytelling")
	void JumpToNode();

	virtual void Activate() override;
	virtual void Initialize(const FStoryDataTable& initRow) override;

private:
	bool bStartImmediately;
	
};

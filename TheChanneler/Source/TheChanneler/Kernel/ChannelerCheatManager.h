// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CheatManager.h"
#include "ChannelerCheatManager.generated.h"

class UStoryManager;

/**
 * 
 */
UCLASS(Within = PlayerController)
class THECHANNELER_API UChannelerCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:
	UChannelerCheatManager();
	virtual void InitCheatManager() override;
	
	void SetStoryManager(UStoryManager& storyManager);

	UFUNCTION(Exec, Category = "Storytelling")
	void SkipCurrentStoryNode();

private:
	UStoryManager* mStoryManager;
};

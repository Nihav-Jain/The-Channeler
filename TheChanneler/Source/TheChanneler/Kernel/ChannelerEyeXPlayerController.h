// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EyeXPlayerController.h"
#include "ChannelerEyeXPlayerController.generated.h"

class UStoryManager;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class THECHANNELER_API AChannelerEyeXPlayerController : public AEyeXPlayerController
{
	GENERATED_BODY()
	
public:
	AChannelerEyeXPlayerController();
	virtual void BeginPlay() override;
	
	UChannelerCheatManager& CheatManager();

	void SetStoryManager(UStoryManager& storyManager);

	UFUNCTION(Exec, Category = "Storytelling")
	void SkipCurrentStoryNode();

	UFUNCTION(Exec, Category = "Storytelling")
	void TransitionToDialogueCam();

	UFUNCTION(Exec, Category = "Storytelling")
	void TransferControlToPlayer();

	UFUNCTION(Exec, BlueprintCallable, Category = "Storytelling")
	void JumpToStoryNode(FString nodeName);

private:
	class UChannelerCheatManager* mCheatManager;
	UStoryManager* mStoryManager;
};

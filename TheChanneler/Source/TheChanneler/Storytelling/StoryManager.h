// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "Components/ActorComponent.h"
#include "StoryManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoryParsed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoryManagerActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoryManagerDeactivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoryBegun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoryEnded);

class UStoryNode;

/**
 * 
 */
UCLASS(abstract, BlueprintType, Blueprintable)
class THECHANNELER_API UStoryManager : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UStoryManager();

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FStoryManagerActivated StoryManagerActivated;

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FStoryManagerDeactivated StoryManagerDeactivated;

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FStoryBegun StoryBegun;

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FStoryEnded StoryEnded;

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FStoryParsed StoryParsed;

	UStoryNode* AddNextNode(UClass* className, const FString& nodeName);

	UFUNCTION(BlueprintCallable, Category = "Storytelling")
	virtual void GotoNextNode();

	UFUNCTION(BlueprintCallable, Category = "Storytelling")
	void BeginStory();

	UFUNCTION(BlueprintCallable, Category = "Storytelling")
	void ActivateStoryManager();

	UFUNCTION(BlueprintCallable, Category = "Storytelling")
	void DeactivateStoryManager();

	virtual void SkipNode();

	UFUNCTION(BlueprintCallable, Category = "Storytelling")
	UStoryNode* GetNodeByName(FString nodeName) const;

	UFUNCTION(BlueprintCallable, Category="Storytelling")
	UStoryNode* GetCurrentNode() const;

	void SetCurrentNode(UStoryNode* node, bool activate = false);

protected:
	bool bIsActive;

	UPROPERTY()
	UStoryNode* mRootStoryNode;
	UStoryNode* mCurrentNode;

private:
	TMap<FString, UStoryNode*> mNodeMap;
};

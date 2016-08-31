// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Storytelling/StoryNode.h"
#include "StorySceneNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartScene);

class UStoryManager;
/**
 * 
 */
UCLASS(BlueprintType)
class THECHANNELER_API UStorySceneNode : public UStoryNode
{
	GENERATED_BODY()
	
public:
	UStorySceneNode();

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FStartScene StartScene;

	virtual void Activate() override;
	virtual void Initialize(const FStoryDataTable& initRow) override;

	UFUNCTION(BlueprintCallable, Category="Storytelling")
	void SetSceneComplete();

private:
	bool mAutoComplete;

};

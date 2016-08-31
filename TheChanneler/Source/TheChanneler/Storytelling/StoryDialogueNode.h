// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Storytelling/StoryNode.h"
#include "StoryDataTable.h"
#include "StoryDialogueNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDialogueStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDialogueEnded);


/**
 * 
 */
UCLASS(BlueprintType)
class THECHANNELER_API UStoryDialogueNode : public UStoryNode
{
	GENERATED_BODY()
	
public:
	UStoryDialogueNode();
	~UStoryDialogueNode();

	UPROPERTY(BlueprintReadOnly, Category = "Storytelling")
	FString Speaker;
	
	UPROPERTY(BlueprintReadOnly, Category = "Storytelling")
	FString Dialogue;
	
	UPROPERTY(BlueprintReadOnly, Category = "Storytelling")
	USoundWave* DialogueSoundWave;

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FDialogueStarted DialogueStarted;

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FDialogueEnded DialogueEnded;

	UPROPERTY()
	class UDialogueComponent* DialogueWidget;

	virtual void Initialize(const FStoryDataTable& initRow) override;

	virtual void Activate() override;
	virtual bool Deactivate() override;

private:
	void PopulateDialogueComponents();

	static TMap<FString, UDialogueComponent*> sSpeakerNameMap;
};

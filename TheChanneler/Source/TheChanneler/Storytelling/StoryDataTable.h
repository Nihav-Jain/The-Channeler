// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "StoryDataTable.generated.h"

UENUM(BlueprintType)
enum class EStoryDialogueNodeType : uint8
{
	NONE,
	DIALOGUE,
	CONDITION,
	SCENE
};

USTRUCT(BlueprintType)
struct FStoryDataTable : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
	FString NodeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
	FString Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
	FString Dialogue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
	TSubclassOf<class UStoryNode> NodeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
	USoundWave* DialogueAudio;
};


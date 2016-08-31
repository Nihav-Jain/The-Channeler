// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Storytelling/StoryNode.h"
#include "StoryDialogueFreezeCamNode.generated.h"

class ADialogueFreezeCam;

//UENUM(BlueprintType)
//enum class EDialogueFreezeCamNodeStates : uint8
//{
//	TO
//};

/**
 * ToDialogueCam
 * ToPuzzleCam  
 * BackToPlayer
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAboutToStartPuzzle);


UCLASS(BlueprintType)
class THECHANNELER_API UStoryDialogueFreezeCamNode : public UStoryNode
{
	GENERATED_BODY()
	
public:
	UStoryDialogueFreezeCamNode();
	~UStoryDialogueFreezeCamNode();
	
	virtual void Initialize(const FStoryDataTable& initRow) override;
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FAboutToStartPuzzle AboutToStartPuzzle;

	UPROPERTY(BlueprintReadOnly, Category="Storytelling")
	ADialogueFreezeCam* mFreezeCam;

private:
	void PopulateDialogueFreezeCams();
	
	UFUNCTION()
	void CameraTransitionComplete(EDialogueCamState oldState, EDialogueCamState newState);

	FString mAction;
	
	FScriptDelegate mCameraTransitionDelegate;

	static TMap<FString, ADialogueFreezeCam*> sCamMap;
};

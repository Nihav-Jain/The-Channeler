// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "StoryDataTable.h"
#include "StoryNode.generated.h"

/**
 * 
 */
UCLASS(abstract, BlueprintType, Blueprintable)
class THECHANNELER_API UStoryNode : public UObject
{
	GENERATED_BODY()
	
public:
	UStoryNode();

	void AddNextNode(UStoryNode* next);

	const UStoryNode& NextNode() const;
	UStoryNode& NextNode();

	virtual void Activate() {};
	virtual bool Deactivate() { return true; };

	bool RequiresTick() const;
	bool CanMoveToNext() const;
	void UpdateCanMoveToNext(bool value);

	virtual void Initialize(const FStoryDataTable& initRow);

	UFUNCTION(BlueprintImplementableEvent, Category = "Storytelling")
	void Init(const FStoryDataTable& initRow);

	void SetWorld(UWorld& world);

	UFUNCTION(BlueprintCallable, Category = "Storytelling")
	const FString& NodeName() const;
	
	UPROPERTY(BlueprintReadOnly, Category = "Storytelling")
	class UStoryManager* Manager;

protected:
	bool bRequiresTick;
	bool bCanMoveToNext;

	UPROPERTY()
	UStoryNode* mNextNode;
	UWorld* mWorld;

	FString mNodeName;
	class UStoryManager* mManager;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Storytelling/StoryManager.h"
#include "Engine/DataTable.h"
#include "StoryDataTable.h"
#include "StoryDialogueManager.generated.h"


/**
 * 
 */
UCLASS(ClassGroup = (Storytelling), meta = (BlueprintSpawnableComponent))
class THECHANNELER_API UStoryDialogueManager : public UStoryManager
{
	GENERATED_BODY()
	
public:
	UStoryDialogueManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	UDataTable* StoryDataTable;

	UFUNCTION(BlueprintCallable, Category = "Storytelling")
	void Update(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Storytelling")
	virtual	void GotoNextNode() override;

	virtual void SkipNode() override;

private:
	bool bIsFirstTime;
};

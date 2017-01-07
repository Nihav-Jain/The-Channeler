// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "../Kernel/GameInstanceBase.h"
#include "../Analytics/Analytics.h"
#include "../Storytelling/StoryManager.h"
#include "ChannelerUtils.generated.h"

class AChannelerCharacter;
class AChannelerEyeXPlayerController;
class UInputDeviceManager;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class THECHANNELER_API UChannelerUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:	
	~UChannelerUtils();

	UFUNCTION(BlueprintCallable, Category = "ChannelerUtils")
	static UGameInstanceBase* GetChannelerGameInstance();

	UFUNCTION(BlueprintCallable, Category = "ChannelerUtils")
	static UAnalytics* GetChannelerAnalytics();

	UFUNCTION(BlueprintCallable, Category = "ChannelerUtils")
	static void SetStoryManager(UStoryManager* manager);

	UFUNCTION(BlueprintCallable, Category = "ChannelerUtils")
	static UStoryManager* GetStoryManager();

	UFUNCTION(BlueprintCallable, Category = "ChannelerUtils")
	static AChannelerCharacter* GetChanneler();

	UFUNCTION(BlueprintCallable, Category = "ChannelerUtils")
	static AChannelerEyeXPlayerController* GetChannelerPlayerController();

	UFUNCTION(BlueprintCallable, Category = "ChannelerUtils")
	static UInputDeviceManager* GetInputDeviceManager();

private:
	static void SetGameInstance(UGameInstanceBase* instance);
	static void SetChanneler(AChannelerCharacter* channeler);
	static void SetChannelerPlayerController(AChannelerEyeXPlayerController* channeler);

	static UGameInstanceBase* GameInstanceBase;
	static UStoryManager* StoryManager;
	static AChannelerCharacter* ChannelerCharacter;
	static AChannelerEyeXPlayerController* ChannelerPlayerController;

	friend class UGameInstanceBase;
	friend class AChannelerCharacter;
	friend class AChannelerEyeXPlayerController;
};

// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../Characters/ChannelerCharacter.h"
#include "../Kernel/ChannelerEyeXPlayerController.h"
#include "ChannelerUtils.h"
#include "../Input/InputDeviceManager.h"
#include "../Kernel/ChannelerPlayerController.h"

UGameInstanceBase* UChannelerUtils::GameInstanceBase;
UStoryManager* UChannelerUtils::StoryManager;
AChannelerCharacter* UChannelerUtils::ChannelerCharacter;
AChannelerEyeXPlayerController* UChannelerUtils::ChannelerPlayerController;

UChannelerUtils::~UChannelerUtils()
{
	GameInstanceBase = nullptr;
	StoryManager = nullptr;
	ChannelerCharacter = nullptr;
	ChannelerPlayerController = nullptr;
}

UGameInstanceBase* UChannelerUtils::GetChannelerGameInstance()
{
	return GameInstanceBase;
}

UAnalytics* UChannelerUtils::GetChannelerAnalytics()
{
	if (GameInstanceBase == nullptr)
		return nullptr;
	return GameInstanceBase->Analytics;
}

void UChannelerUtils::SetStoryManager(UStoryManager* manager)
{
	StoryManager = manager;
}

UStoryManager* UChannelerUtils::GetStoryManager()
{
	return StoryManager;
}

void UChannelerUtils::SetGameInstance(UGameInstanceBase* instance)
{
	GameInstanceBase = instance;
}

AChannelerCharacter* UChannelerUtils::GetChanneler()
{
	return ChannelerCharacter;
}

void UChannelerUtils::SetChanneler(AChannelerCharacter* channeler)
{
	ChannelerCharacter = channeler;
}

AChannelerEyeXPlayerController* UChannelerUtils::GetChannelerPlayerController()
{
	return ChannelerPlayerController;
}


void UChannelerUtils::SetChannelerPlayerController(AChannelerEyeXPlayerController* playerController)
{
	ChannelerPlayerController = playerController;
}

UInputDeviceManager* UChannelerUtils::GetInputDeviceManager()
{
	if (GEngine != nullptr)
	{
		if (GEngine->GetWorld() != nullptr)
		{
			APlayerController* controller = GEngine->GetWorld()->GetFirstPlayerController();
			AChannelerPlayerController* channelerController = Cast<AChannelerPlayerController>(controller);
			if (channelerController == nullptr)
			{
				AChannelerEyeXPlayerController* channelerEyeXController = Cast<AChannelerEyeXPlayerController>(controller);
				if (channelerEyeXController != nullptr)
				{
					return channelerEyeXController->InputDeviceManager;
				}
			}
			else
			{
				return channelerController->InputDeviceManager;
			}
		}
	}
	return nullptr;
}

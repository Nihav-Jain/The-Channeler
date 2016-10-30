// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../Analytics/Analytics.h"
#include "../Utils/ChannelerUtils.h"
#include "../UI/ChannelerAudioComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "GameInstanceBase.h"


UGameInstanceBase::UGameInstanceBase() :
	Analytics(nullptr), CurrentScreenResolution(1280, 720)
{}

void UGameInstanceBase::Init()
{
	LoadAvailableScreenResolutions();
	Analytics = NewObject<UAnalytics>(this, UAnalytics::StaticClass());
	Super::Init();
	UChannelerUtils::SetGameInstance(this);
#if !WITH_EDITOR
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UGameInstanceBase::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMap.AddUObject(this, &UGameInstanceBase::EndLoadingScreen);
	if (LoadingScreenClass) {
		LoadingScreen = CreateWidget<UUserWidget>(this, LoadingScreenClass);
	}
#endif
}

void UGameInstanceBase::BeginLoadingScreen()
{
	if (LoadingScreenClass) {

		FLoadingScreenAttributes loadingScreenAttributes;
		loadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = false;
		loadingScreenAttributes.WidgetLoadingScreen = LoadingScreen->TakeWidget();

		GetMoviePlayer()->SetupLoadingScreen(loadingScreenAttributes);
	}
}

void UGameInstanceBase::EndLoadingScreen()
{

}

void UGameInstanceBase::SetNewScreenResolution(const FIntPoint& newScreenResolution)
{
#if !WITH_EDITOR
	if (GEngine != nullptr)
	{
		UGameUserSettings* gameSettings = GEngine->GetGameUserSettings();
		if (gameSettings != nullptr)
		{
			LoadAvailableScreenResolutions();
			int32 index = 0;
			if (!AvailableScreenResolutions.Find(newScreenResolution, index))
			{
				UE_LOG(LogTemp, Error, TEXT("Requested screen resolution %d x %d is not available on this device."), newScreenResolution.X, newScreenResolution.Y);
				return;
			}

			bool applySettings = false;

			if (gameSettings->GetFullscreenMode() != EWindowMode::Fullscreen)
			{
				gameSettings->SetWindowPosition(-1, -1);
				gameSettings->SetFullscreenMode(EWindowMode::Fullscreen);
				applySettings = true;
			}
			FIntPoint currentScreenRes = gameSettings->GetScreenResolution();
			if (newScreenResolution.X != currentScreenRes.X || newScreenResolution.Y != currentScreenRes.Y)
			{
				gameSettings->SetScreenResolution(newScreenResolution);
				CurrentScreenResolution = newScreenResolution;
				UE_LOG(LogTemp, Warning, TEXT("Setting new screen resolution = %d x %d"), newScreenResolution.X, newScreenResolution.Y);
				applySettings = true;
			}

			if (applySettings)
				gameSettings->ApplySettings(true);
			gameSettings->SaveSettings();
		}
	}
#endif
}

void UGameInstanceBase::LoadAvailableScreenResolutions()
{
	if (AvailableScreenResolutions.Num() == 0)
	{
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(AvailableScreenResolutions);

		for (int32 i = 0; i < AvailableScreenResolutions.Num(); i++)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d x %d"), AvailableScreenResolutions[i].X, AvailableScreenResolutions[i].Y);
		}
	}
}

void UGameInstanceBase::ReinforceScreenResolution()
{
	SetNewScreenResolution(CurrentScreenResolution);
}

int32 UGameInstanceBase::GetValidDefaultScreenResolution() const
{
	int32 first1200index = -1;
	for (int32 i = AvailableScreenResolutions.Num() - 1; i >= 0; --i)
	{
		if (AvailableScreenResolutions[i].X < 1300 && AvailableScreenResolutions[i].X >= 1000)
		{
			if(first1200index == -1)
				first1200index = i;

			float aspectRatio = ((float)AvailableScreenResolutions[i].X) / AvailableScreenResolutions[i].Y;
			if (FMath::IsNearlyEqual(aspectRatio, 1.77f, 0.01f))
			{
				return i;
			}
		}
	}

	return (first1200index >= 0) ? first1200index : (AvailableScreenResolutions.Num() - 1);
}


void UGameInstanceBase::LoadAudioChannels()
{
	ChannelerMusicAudioComponentArray.Empty();
	ChannelerVoiceAudioComponentArray.Empty();
	ChannelerSFXAudioComponentArray.Empty();


	UWorld* world = GetWorld();
	if (world == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("WRODL IS NULL"));
		return;
	}

	TSubclassOf<UChannelerAudioComponent> audioComponentSubclass(UChannelerAudioComponent::StaticClass());

	for (TActorIterator<AActor> ActorItr(world); ActorItr; ++ActorItr)
	{
		AActor* actor = *ActorItr;
		if (actor == nullptr)
			continue;
		TArray<UActorComponent*> components = actor->GetComponentsByClass(audioComponentSubclass);
		for (int32 index = 0; index < components.Num(); index++)
		{ 
			if (components[index] != nullptr)
			{
				UChannelerAudioComponent* audioComponent = Cast<UChannelerAudioComponent>(components[index]);
				if (audioComponent != nullptr)
				{
					switch (audioComponent->AudioChannel)
					{
					case UChannelerAudioComponent::Channel::Music:
						ChannelerMusicAudioComponentArray.Add(audioComponent);
						break;
					case UChannelerAudioComponent::Channel::SFX:
						ChannelerSFXAudioComponentArray.Add(audioComponent);
						break;
					case UChannelerAudioComponent::Channel::Voice:
						ChannelerVoiceAudioComponentArray.Add(audioComponent);
						break;
					}
				}
			}
		}
	}

}

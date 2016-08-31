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
	Analytics(nullptr)
{}

void UGameInstanceBase::Init()
{
	UpdateScreenResolution();
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

void UGameInstanceBase::UpdateScreenResolution()
{
#if !WITH_EDITOR
	if (GEngine != nullptr)
	{
		UGameUserSettings* gameSettings = GEngine->GetGameUserSettings();
		if (gameSettings != nullptr)
		{
			TArray<FIntPoint> availableResolutions;
			bool resolutionQuery = UKismetSystemLibrary::GetSupportedFullscreenResolutions(availableResolutions);

			bool applySettings = false;

			if (gameSettings->GetFullscreenMode() != EWindowMode::Fullscreen)
			{
				gameSettings->SetWindowPosition(-1, -1);
				gameSettings->SetFullscreenMode(EWindowMode::Fullscreen);
				applySettings = true;
			}
			if (resolutionQuery)
			{
				FIntPoint currentScreenRes = gameSettings->GetScreenResolution();
				const FIntPoint& maxScreenRes = availableResolutions.Last();
				if (maxScreenRes.X != currentScreenRes.X || maxScreenRes.Y != currentScreenRes.Y)
				{
					gameSettings->SetScreenResolution(maxScreenRes);
					applySettings = true;
				}
			}
			if (applySettings)
				gameSettings->ApplySettings(true);
			//gameSettings->SaveSettings();
		}
	}
#endif
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
		UActorComponent* component = actor->GetComponentByClass(audioComponentSubclass);
		if (component != nullptr)
		{
			UChannelerAudioComponent* audioComponent = Cast<UChannelerAudioComponent>(component);
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

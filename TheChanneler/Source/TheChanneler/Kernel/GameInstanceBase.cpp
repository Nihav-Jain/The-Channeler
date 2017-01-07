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
	Analytics(nullptr), CurrentScreenResolution(-1, -1)
{}

void UGameInstanceBase::Init()
{
	LoadAvailableScreenResolutions();
	Analytics = NewObject<UAnalytics>(this, UAnalytics::StaticClass());
	
	Super::Init();
	UChannelerUtils::SetGameInstance(this);

	if (AvailableScreenResolutions.Find(CurrentScreenResolution) == INDEX_NONE)
	{
		CurrentScreenResolution = GetValidDefaultScreenResolution();
	}

	SetNewScreenResolution(CurrentScreenResolution);

#if BUILD_SHIPPING
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
	if (GEngine != nullptr)
	{
		UGameUserSettings* gameSettings = GEngine->GetGameUserSettings();
		if (gameSettings != nullptr)
		{
			LoadAvailableScreenResolutions();
			int32 index = 0;

			bool applySettings = false;
			if (!AvailableScreenResolutions.Find(newScreenResolution, index))
			{
				UE_LOG(LogTemp, Error, TEXT("Requested screen resolution %d x %d is not available on this device."), newScreenResolution.X, newScreenResolution.Y);
			}
			else
			{
				FIntPoint currentScreenRes = gameSettings->GetScreenResolution();
				CurrentScreenResolutionIndex = index;
				if (newScreenResolution.X != currentScreenRes.X || newScreenResolution.Y != currentScreenRes.Y)
				{
					
#if !WITH_EDITOR
					gameSettings->SetScreenResolution(newScreenResolution);
#endif

					CurrentScreenResolution = newScreenResolution;
					UE_LOG(LogTemp, Warning, TEXT("Setting new screen resolution = %d x %d"), newScreenResolution.X, newScreenResolution.Y);
					applySettings = true;
				}
			}

#if !WITH_EDITOR
			if (gameSettings->GetFullscreenMode() != EWindowMode::Fullscreen)
			{
				gameSettings->SetWindowPosition(-1, -1);
				gameSettings->SetFullscreenMode(EWindowMode::Fullscreen);
				applySettings = true;
			}

			if (applySettings)
			{
				gameSettings->ApplySettings(true);
			}

			gameSettings->SaveSettings();
#endif
		}
	}
}

void UGameInstanceBase::SetNewScreenResolutionByIndex(int32 newScreenResolutionIndex)
{
	if (newScreenResolutionIndex >= 0 && newScreenResolutionIndex < AvailableScreenResolutions.Num())
	{
		SetNewScreenResolution(AvailableScreenResolutions[newScreenResolutionIndex]);
	}
	else
	{
		SetNewScreenResolution(GetValidDefaultScreenResolution());
	}
}

void UGameInstanceBase::LoadAvailableScreenResolutions()
{
	if (AvailableScreenResolutions.Num() == 0)
	{
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(AvailableScreenResolutions);

		for (int32 i = 0; i < AvailableScreenResolutions.Num(); i++)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d x %d"), AvailableScreenResolutions[i].X, AvailableScreenResolutions[i].Y);
			VerbalSupportedScreenResolutions.Add(FString::Printf(TEXT("%dx%d"), AvailableScreenResolutions[i].X, AvailableScreenResolutions[i].Y));
		}
	}
}

void UGameInstanceBase::ReinforceScreenResolution()
{
	SetNewScreenResolution(CurrentScreenResolution);
}

FIntPoint UGameInstanceBase::GetValidDefaultScreenResolution() const
{
	FDisplayMetrics displayMetrics;
	FDisplayMetrics::GetDisplayMetrics(displayMetrics);
	
	//for (int32 i = AvailableScreenResolutions.Num() - 1; i >= 0; --i)
	//{
	//	if (AvailableScreenResolutions[i].X == displayMetrics.PrimaryDisplayWidth 
	//		&& AvailableScreenResolutions[i].Y == displayMetrics.PrimaryDisplayHeight)
	//	{
	//		return i;
	//	}
	//}

	return FIntPoint(displayMetrics.PrimaryDisplayWidth, displayMetrics.PrimaryDisplayHeight);
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

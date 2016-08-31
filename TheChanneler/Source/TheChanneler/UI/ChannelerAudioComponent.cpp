// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "Engine.h"
#include "../Kernel/GameInstanceBase.h"
#include "../Utils/ChannelerUtils.h"
#include "ChannelerAudioComponent.h"
#include "Audio.h"
#include "SoundDefinitions.h"
#include "Sound/SoundNodeAttenuation.h"
#include "Sound/SoundCue.h"
#include "SubtitleManager.h"


UChannelerAudioComponent::UChannelerAudioComponent()
{
}

void UChannelerAudioComponent::Activate(bool bReset)
{
	UAudioComponent::Activate(bReset);

	/*if (AddedMe > 1)
		return;

		UGameInstanceBase* gameInstance = nullptr;

		UWorld* tempWorld = GetWorld();		

		if (tempWorld == nullptr)
			return;

		UGameInstance* instance = tempWorld->GetGameInstance();
		
		if (instance == nullptr)
			return;

		gameInstance = Cast<UGameInstanceBase>(instance);
		if (gameInstance == nullptr)
			return;

		switch (AudioChannel)
		{
		case Channel::Music:
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Music")));
			gameInstance->ChannelerMusicAudioComponentArray.Add(this);
			break;
		case Channel::Voice:
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Voice")));
			gameInstance->ChannelerVoiceAudioComponentArray.Add(this);
			break;
		case Channel::SFX:
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("SFX")));
			gameInstance->ChannelerSFXAudioComponentArray.Add(this);
			break;
		default:
			break;
		}
		AddedMe = 2;*/
	
}

void UChannelerAudioComponent::Deactivate()
{
	UAudioComponent::Deactivate();

	/*if (AddedMe < 1)
		return;

	UGameInstanceBase* gameInstance = UChannelerUtils::GetChannelerGameInstance();
	if (gameInstance == nullptr)
		return;
	switch (AudioChannel)
	{
	case Channel::Music:
		gameInstance->ChannelerMusicAudioComponentArray.Remove(this);
		break;
	case Channel::Voice:
		gameInstance->ChannelerVoiceAudioComponentArray.Remove(this);
		break;
	case Channel::SFX:
		gameInstance->ChannelerSFXAudioComponentArray.Remove(this);
		break;
	default:
		break;
	}*/
}

void UChannelerAudioComponent::FadeInMute(float FadeInDuration, float FadeVolumeLevel, float StartTime)
{
	if (FAudioDevice* AudioDevice = GetAudioDevice())
	{
		FActiveSound* ActiveSound = AudioDevice->FindActiveSound(this);
		if (ActiveSound)
		{
			StartTime = ActiveSound->PlaybackTime;
			PlayInternal(StartTime, FadeInDuration, FadeVolumeLevel);
		}
		else
		{
			PlayInternal(StartTime, FadeInDuration, FadeVolumeLevel);
		}
	}
}

void UChannelerAudioComponent::FadeOutMute(float FadeOutDuration, float FadeVolumeLevel)
{
	if (bIsActive)
	{
		if (FadeOutDuration > 0.0f)
		{
			// TODO - Audio Threading. This call would be a task
			if (FAudioDevice* AudioDevice = GetAudioDevice())
			{
				FActiveSound* ActiveSound = AudioDevice->FindActiveSound(this);
				if (ActiveSound)
				{
					ActiveSound->TargetAdjustVolumeMultiplier = FadeVolumeLevel;
					ActiveSound->TargetAdjustVolumeStopTime = ActiveSound->PlaybackTime + FadeOutDuration;
					//ActiveSound->bFadingOut = true;

				}
			}
		}
		else
		{
			//Stop();
		}
	}
}

void UChannelerAudioComponent::PlayInternal(const float StartTime, const float FadeInDuration, const float FadeVolumeLevel)
{
	if (FAudioDevice* AudioDevice = GetAudioDevice())
	{
		FActiveSound* ActiveSound = AudioDevice->FindActiveSound(this);
		if(ActiveSound)
		{
			ActiveSound->TargetAdjustVolumeMultiplier = FadeVolumeLevel;

			if (FadeInDuration > 0.0f)
			{
				ActiveSound->CurrentAdjustVolumeMultiplier = 0.f;
				ActiveSound->TargetAdjustVolumeStopTime = ActiveSound->PlaybackTime + FadeInDuration;
			}
			else
			{
				ActiveSound->CurrentAdjustVolumeMultiplier = FadeVolumeLevel;
			}
		}
		else
		{
			UWorld* World = GetWorld();

			UE_LOG(LogAudio, Verbose, TEXT("%g: Playing AudioComponent : '%s' with Sound: '%s'"), World ? World->GetAudioTimeSeconds() : 0.0f, *GetFullName(), Sound ? *Sound->GetName() : TEXT("nullptr"));

			if (bIsActive)
			{
				// If this is an auto destroy component we need to prevent it from being auto-destroyed since we're really just restarting it
				bool bCurrentAutoDestroy = bAutoDestroy;
				bAutoDestroy = false;
				Stop();
				bAutoDestroy = bCurrentAutoDestroy;
			}

			if (Sound && (World == nullptr || World->bAllowAudioPlayback))
			{
					FActiveSound NewActiveSound;
					NewActiveSound.SetAudioComponent(this);
					NewActiveSound.World = GetWorld();
					NewActiveSound.Sound = Sound;
					NewActiveSound.SoundClassOverride = SoundClassOverride;

					NewActiveSound.VolumeMultiplier =  VolumeMultiplier;
					NewActiveSound.PitchMultiplier =  PitchMultiplier;
					NewActiveSound.HighFrequencyGainMultiplier = HighFrequencyGainMultiplier;

					NewActiveSound.RequestedStartTime = FMath::Max(0.f, StartTime);
					NewActiveSound.OcclusionCheckInterval = OcclusionCheckInterval;
					NewActiveSound.SubtitlePriority = SubtitlePriority;

					NewActiveSound.bShouldRemainActiveIfDropped = bShouldRemainActiveIfDropped;
					NewActiveSound.bHandleSubtitles = (!bSuppressSubtitles || OnQueueSubtitles.IsBound());
					NewActiveSound.bIgnoreForFlushing = bIgnoreForFlushing;

					NewActiveSound.bIsUISound = bIsUISound;
					NewActiveSound.bIsMusic = bIsMusic;
					NewActiveSound.bAlwaysPlay = bAlwaysPlay;
					NewActiveSound.bReverb = bReverb;
					NewActiveSound.bCenterChannelOnly = bCenterChannelOnly;

					NewActiveSound.bLocationDefined = !bPreviewComponent;
					if (NewActiveSound.bLocationDefined)
					{
						NewActiveSound.Transform = ComponentToWorld;
					}

					const FAttenuationSettings* AttenuationSettingsToApply = (bAllowSpatialization ? GetAttenuationSettingsToApply() : nullptr);
					NewActiveSound.bAllowSpatialization = bAllowSpatialization;
					NewActiveSound.bHasAttenuationSettings = (AttenuationSettingsToApply != nullptr);
					if (NewActiveSound.bHasAttenuationSettings)
					{
						NewActiveSound.AttenuationSettings = *AttenuationSettingsToApply;
					}

					NewActiveSound.InstanceParameters = InstanceParameters;

					NewActiveSound.TargetAdjustVolumeMultiplier = FadeVolumeLevel;

					if (FadeInDuration > 0.0f)
					{
						NewActiveSound.CurrentAdjustVolumeMultiplier = 0.f;
						NewActiveSound.TargetAdjustVolumeStopTime = FadeInDuration;
					}
					else
					{
						NewActiveSound.CurrentAdjustVolumeMultiplier = FadeVolumeLevel;
					}

					// TODO - Audio Threading. This call would be a task call to dispatch to the audio thread
					AudioDevice->AddNewActiveSound(NewActiveSound);

					bIsActive = true;
			}
		}
	}
}

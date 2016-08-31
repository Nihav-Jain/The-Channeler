// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/AudioComponent.h"
#include "ChannelerAudioComponent.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class THECHANNELER_API UChannelerAudioComponent : public UAudioComponent
{
	GENERATED_BODY()



public:

		UChannelerAudioComponent();

		UENUM(BlueprintType)		//"BlueprintType" is essential to include
		enum class Channel : uint8
		{
			Music 	UMETA(DisplayName = "Music"),
			Voice 	UMETA(DisplayName = "Voice"),
			SFX		UMETA(DisplayName = "SFX")
		};

		virtual void Activate(bool bReset = false) override;
		virtual void Deactivate() override;

		UFUNCTION(BlueprintCallable, Category = "Audio|Components|Audio")
		void FadeInMute(float FadeInDuration, float FadeVolumeLevel = 1.f, float StartTime = 0.f);

		UFUNCTION(BlueprintCallable, Category = "Audio|Components|Audio")
		void FadeOutMute(float FadeOutDuration, float FadeVolumeLevel);

		
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum)
		Channel AudioChannel = Channel::Music;
		int AddedMe = 0;

private:

	/** Utility function called by Play and FadeIn to start a sound playing. */
	void PlayInternal(const float StartTime = 0.f, const float FadeInDuration = 0.f, const float FadeVolumeLevel = 1.f);



	
};

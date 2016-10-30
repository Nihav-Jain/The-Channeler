// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "MoviePlayer.h"
#include "GameInstanceBase.generated.h"

class UChannelerAudioComponent;

/**
 * The default game instance for The Channeler.
 */
UCLASS()
class THECHANNELER_API UGameInstanceBase : public UGameInstance
{
	GENERATED_BODY()

public:

	UGameInstanceBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioComponent")
		TArray<UChannelerAudioComponent*> ChannelerMusicAudioComponentArray;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioComponent")
		TArray<UChannelerAudioComponent*> ChannelerVoiceAudioComponentArray;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioComponent")
		TArray<UChannelerAudioComponent*> ChannelerSFXAudioComponentArray;

	/** The user's profile name as a string */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = User)
	FString UserName;

	/** The user's profile as a string */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = User)
	FString UserProfile;
	
	UPROPERTY(BlueprintReadOnly, Category = "Analytics")
	class UAnalytics* Analytics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadingScreen")
	TSubclassOf<class UUserWidget> LoadingScreenClass;

	UFUNCTION(BlueprintCallable, Category = "Channeler Audio")
	void LoadAudioChannels();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetNewScreenResolution(const FIntPoint& newScreenResolution);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings")
	TArray<FIntPoint> AvailableScreenResolutions;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ReinforceScreenResolution();

	virtual void Init() override;

	void BeginLoadingScreen();
	void EndLoadingScreen();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings")
	FIntPoint CurrentScreenResolution;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	int32 GetValidDefaultScreenResolution() const;

private:
	UFUNCTION()
	void LoadAvailableScreenResolutions();

	UPROPERTY()
	UUserWidget * LoadingScreen;
};

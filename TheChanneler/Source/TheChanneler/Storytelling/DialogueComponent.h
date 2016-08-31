// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/WidgetComponent.h"
#include "DialogueComponent.generated.h"

USTRUCT(BlueprintType)
struct FTextAnimation
{
	GENERATED_BODY()

public:
	FTextAnimation();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = ( ClampMin="0.01" ))
	float TimeStep;
};

class UChannelerAudioComponent;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Storytelling), meta = (BlueprintSpawnableComponent))
class THECHANNELER_API UDialogueComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UDialogueComponent();

	UPROPERTY(EditAnywhere, Category = "Storytelling")
	FString SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling", meta = (ClampMin="2"))
	int32 FontSize;
	
	UPROPERTY(BlueprintReadOnly, Category="Storytelling")
	FString Dialogue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
	bool Animate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storytelling")
	FTextAnimation AnimationProperties;

	UPROPERTY(BlueprintReadOnly, Category = "Storytelling")
	UChannelerAudioComponent* OwnerAudioComponent;

	UFUNCTION(BlueprintNativeEvent, Category = "Storytelling")
	void ShowDialogue();

	UFUNCTION(BlueprintNativeEvent, Category = "Storytelling")
	void HideDialogue();

	UFUNCTION(BlueprintNativeEvent, Category = "Storytelling")
	void SkipAnimation();

	void PlaySound(USoundWave& soundWave);
	void StopSound();

	bool IsAnimating() const;

protected:
	UPROPERTY(BlueprintReadWrite, Category="Storytelling")
	bool bIsAnimating;
};

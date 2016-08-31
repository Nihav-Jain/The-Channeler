// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "DialogueComponent.h"
#include "../UI/ChannelerAudioComponent.h"

FTextAnimation::FTextAnimation() :
	TimeStep(0.05f)
{}

UDialogueComponent::UDialogueComponent() :
	FontSize(24), bIsAnimating(false), Animate(true),
	AnimationProperties()
{}

void UDialogueComponent::ShowDialogue_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowDialogue: %s"), *Dialogue);
	// TODO:
}

void UDialogueComponent::HideDialogue_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("HideDialogue"));
	// TODO
}

void UDialogueComponent::SkipAnimation_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("SkipAnimation"));
	bIsAnimating = false;
	// TODO
}

void UDialogueComponent::PlaySound(USoundWave& soundWave)
{
	if (OwnerAudioComponent != nullptr)
	{
		StopSound();
		OwnerAudioComponent->SetSound(&soundWave);
		OwnerAudioComponent->Play();
	}
}

void UDialogueComponent::StopSound()
{
	if (OwnerAudioComponent != nullptr)
	{
		if (OwnerAudioComponent->IsPlaying())
			OwnerAudioComponent->Stop();
	}
}

bool UDialogueComponent::IsAnimating() const
{
	return bIsAnimating;
}
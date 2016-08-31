// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "DialogueComponent.h"
#include "../UI/ChannelerAudioComponent.h"
#include "StoryDialogueNode.h"

TMap<FString, UDialogueComponent*> UStoryDialogueNode::sSpeakerNameMap;

UStoryDialogueNode::UStoryDialogueNode() :
	Speaker(""), Dialogue(""), DialogueWidget(nullptr), DialogueSoundWave(nullptr),
	DialogueStarted(), DialogueEnded()
{}

UStoryDialogueNode::~UStoryDialogueNode()
{
	if (sSpeakerNameMap.Num() != 0)
		sSpeakerNameMap.Empty();
}

void UStoryDialogueNode::Activate()
{
	if (DialogueWidget != nullptr)
	{
		DialogueWidget->Dialogue = Dialogue;
		DialogueWidget->ShowDialogue();
		if (DialogueSoundWave != nullptr)
			DialogueWidget->PlaySound(*DialogueSoundWave);
	}
	if (DialogueStarted.IsBound())
		DialogueStarted.Broadcast();
}

bool UStoryDialogueNode::Deactivate()
{
	if (DialogueWidget != nullptr)
	{
		if (DialogueWidget->IsAnimating())
		{
			DialogueWidget->SkipAnimation();
			return  false;
		}
		DialogueWidget->HideDialogue();
		DialogueWidget->StopSound();
	}
	if (DialogueEnded.IsBound())
		DialogueEnded.Broadcast();

	return true;
}

void UStoryDialogueNode::Initialize(const FStoryDataTable& initRow)
{
	PopulateDialogueComponents();
	Speaker = initRow.Speaker;
	Dialogue = initRow.Dialogue;
	DialogueSoundWave = initRow.DialogueAudio;

	if (sSpeakerNameMap.Contains(initRow.Speaker))
		DialogueWidget = sSpeakerNameMap[initRow.Speaker];
	else
		UE_LOG(LogTemp, Error, TEXT("Dialogue Component not found: %s"), *initRow.Speaker);

	Super::Initialize(initRow);
}

void UStoryDialogueNode::PopulateDialogueComponents()
{
	if (sSpeakerNameMap.Num() == 0)
	{
		TSubclassOf<UDialogueComponent> dialogueComponentSubclass(UDialogueComponent::StaticClass());
		TSubclassOf<UChannelerAudioComponent> audioComponentSubclass(UChannelerAudioComponent::StaticClass());

		for (TActorIterator<AActor> ActorItr(mWorld); ActorItr; ++ActorItr)
		{
			AActor* actor = *ActorItr;
			if (actor == nullptr)
				continue;
			UActorComponent* component = actor->GetComponentByClass(dialogueComponentSubclass);
			if (component != nullptr)
			{
				UDialogueComponent* dialogueComponent = Cast<UDialogueComponent>(component);
				if (dialogueComponent != nullptr)
				{
					sSpeakerNameMap.Add(dialogueComponent->SpeakerName, dialogueComponent);
					UE_LOG(LogTemp, Warning, TEXT("Actor found with DialogueComponent: %s"), *(dialogueComponent->SpeakerName));

					UActorComponent* audioComponentBase = actor->GetComponentByClass(audioComponentSubclass);
					UChannelerAudioComponent* audioComponent = nullptr;
					if (audioComponentBase != nullptr)
					{
						audioComponent = Cast<UChannelerAudioComponent>(audioComponentBase);
					}
					else
					{
						audioComponent = NewObject<UChannelerAudioComponent>(actor, TEXT("DialogueAudioComponent"));
						audioComponent->AttachTo(actor->GetRootComponent());
					}
					audioComponent->AudioChannel = UChannelerAudioComponent::Channel::Voice;
					audioComponent->Activate(true);
					dialogueComponent->OwnerAudioComponent = audioComponent;
				}
			}
		}
	}
}
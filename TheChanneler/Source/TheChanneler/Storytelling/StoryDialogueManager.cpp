// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../Kernel/ChannelerEyeXPlayerController.h"
#include "../Characters/ChannelerCharacter.h"
#include "StoryDialogueManager.h"


UStoryDialogueManager::UStoryDialogueManager() :
	StoryDataTable(nullptr), bIsFirstTime(true)
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	//RegisterComponent();
}

void UStoryDialogueManager::BeginPlay()
{
	check(StoryDataTable != nullptr);
	UStoryManager::BeginPlay();

	TArray<FName> rowNames = StoryDataTable->GetRowNames();

	for (const FName& rowName : rowNames)
	{
		FStoryDataTable* row = StoryDataTable->FindRow<FStoryDataTable>(rowName, TEXT(""));
		UStoryNode* node = AddNextNode(row->NodeClass.operator UClass*(), row->NodeName);
		node->Initialize(*row);
	}

	mCurrentNode = mRootStoryNode;

	//if (StoryParsed.IsBound())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Story parsed"));
	//	StoryParsed.Broadcast();
	//}

	UWorld* world = GetOwner()->GetWorld();
	if (world != nullptr)
	{
		APlayerController* controller = world->GetFirstPlayerController();
		if (controller != nullptr)
		{
			AChannelerEyeXPlayerController* eyeXController = Cast<AChannelerEyeXPlayerController>(controller);
			if (eyeXController != nullptr)
			{
				eyeXController->SetStoryManager(*this);
			}
		}
	}
}

void UStoryDialogueManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UE_LOG(LogTemp, Warning, TEXT("STickComponen"));
}

void UStoryDialogueManager::Update(float DeltaTime)
{
	if (bIsFirstTime && StoryParsed.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("Story parsed"));
		bIsFirstTime = false;
		StoryParsed.Broadcast();

		Cast<AChannelerCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter())->CallStoryParsed(this);
	}

	if (!bIsActive)
		return;

	if (mCurrentNode->RequiresTick())
	{
		//UE_LOG(LogTemp, Warning, TEXT("condition node"));
		if (mCurrentNode->CanMoveToNext())
		{
			GotoNextNode();
		}
	}
}

void UStoryDialogueManager::GotoNextNode()
{
	if (mCurrentNode != nullptr)
	{
		if (!mCurrentNode->CanMoveToNext())
		{
			return;
		}
	}
	Super::GotoNextNode();
}

void UStoryDialogueManager::SkipNode()
{
	Super::GotoNextNode();
}
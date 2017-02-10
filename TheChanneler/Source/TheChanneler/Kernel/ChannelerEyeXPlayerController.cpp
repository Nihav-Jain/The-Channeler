// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerCheatManager.h"
#include "../Storytelling/StoryNode.h"
#include "../Storytelling/StoryDialogueFreezeCamNode.h"
#include "../Utils/ChannelerUtils.h"
#include "GameInstanceBase.h"
#include "ChannelerEyeXPlayerController.h"
#include "../Input/InputDeviceManager.h"
#include "../TheChannelerGameMode.h"
#include "IEyeXPlugin.h"

AChannelerEyeXPlayerController::AChannelerEyeXPlayerController()
	: mCheatManager(nullptr)
	, InputDeviceManager(nullptr)
	, CheckForClosedEye(false), MinEyeClosedDuration(1.0f)
	, bIsLeftEyeClosed(false), bIsRightEyeClosed(false)
	, mEyeX(nullptr), UserPresence(EEyeXUserPresence::Unknown)
	, mLeftEyeClosedDuration(0.0f), mRightEyeClosedDuration(0.0f)
	, mLastClosedEye(EEyeToDetect::EYE_NONE), mLastTriggeredEyeEvent(EEyeToDetect::EYE_NONE)
	, bDidBlink(false), MinBlinkDuration(0.1f), MaxBlinkDuration(0.5f), mBlinkCount(0), BlinkStreakInterval(0.5)
	, mBlinkTimer(0), bBlinkPossible(false), bMarkedForReset(false), BlinkErrorRange(0.2f), mBlinkErrorTimer(0)
	, MinWinkDuration(0.25f), MaxWinkDuration(0.5f)
	, ExtendedFOVEnabled(true), ExtendedFOVTurnRate(1.0f)
{
	PrimaryActorTick.bCanEverTick = true;

	UChannelerUtils::SetChannelerPlayerController(this);
	InputDeviceManager = NewObject<UInputDeviceManager>(this, UInputDeviceManager::StaticClass(), TEXT("InputDeviceManager"));
	InputDeviceManager->Controller = this;
	UChannelerUtils::SetInputDeviceManager(InputDeviceManager);
}

void AChannelerEyeXPlayerController::BeginPlay()
{
	if (GetWorld() != nullptr)
	{
		if (GetWorld()->GetGameInstance() != nullptr)
		{
			UGameInstanceBase* gameInstance = Cast<UGameInstanceBase>(GetWorld()->GetGameInstance());
			if (gameInstance != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("LoadSettingsIntoChannelerController()"));
				gameInstance->LoadSettingsIntoChannelerController();
			}
		}
	}

	mEyeX = &IEyeXPlugin::Get();
	UserPresence = mEyeX->GetUserPresence();
	
	if (InputDeviceManager == nullptr)
	{
		InputDeviceManager = NewObject<UInputDeviceManager>(this, UInputDeviceManager::StaticClass(), TEXT("InputDeviceManager"));
		UChannelerUtils::SetInputDeviceManager(InputDeviceManager);
	}

	Super::BeginPlay();

	if (mCheatManager == nullptr)
	{
		mCheatManager = NewObject<UChannelerCheatManager>(this, UChannelerCheatManager::StaticClass());
		mCheatManager->InitCheatManager();
	}

	SetActorTickEnabled(true);
	EnableInput(this);

	AGameMode* gameMode = UGameplayStatics::GetGameMode(this);
	if (gameMode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("game mode is null"));
	}
	else
	{
		mGameMode = Cast<ATheChannelerGameMode>(gameMode);
		if (mGameMode == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("channeler game mode is null"));
		}
	}
}

void AChannelerEyeXPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	InputDeviceManager->Tick();
	BlinkWinkTick(DeltaSeconds);
}

void AChannelerEyeXPlayerController::BlinkWinkTick(float deltaSeconds)
{
	TEnumAsByte<EEyeXUserPresence::Type> currentUserPresence = mEyeX->GetUserPresence();

	bool simulateEyeX = IsEyeXSimulating();
	if (!simulateEyeX)
	{
		// check for user presence; if it has changed since last frame, trigger appropriate event
		if (currentUserPresence != UserPresence)
		{
			UserPresence = currentUserPresence;
			switch (currentUserPresence)
			{
			case EEyeXUserPresence::Unknown:		// intentional fall through
			case EEyeXUserPresence::NotPresent:
				if (LostUserPresenceEvent.IsBound())
					LostUserPresenceEvent.Broadcast();
				break;
			case EEyeXUserPresence::Present:
				if (GainedUserPresenceEvent.IsBound())
					GainedUserPresenceEvent.Broadcast();
				break;
			}
		}
	}
	else
	{
		currentUserPresence = EEyeXUserPresence::Present;
	}

	bDidBlink = false;

	if (CheckForClosedEye && currentUserPresence == EEyeXUserPresence::Present)
	{
		FEyeXEyePosition currentEyePosition = mEyeX->GetEyePosition();

		if (!simulateEyeX)
		{
			bIsLeftEyeClosed = !currentEyePosition.bIsLeftEyeValid;
			bIsRightEyeClosed = !currentEyePosition.bIsRightEyeValid;
		}

		if (bMarkedForReset)
			mBlinkErrorTimer += deltaSeconds;

		// both eyes are open
		if (!(bIsLeftEyeClosed || bIsRightEyeClosed))
		{
			/** Wink **/
			// check for left eye wink
			if (mLastClosedEye == EEyeToDetect::EYE_LEFT)
			{
				if (mLeftEyeClosedDuration >= MinWinkDuration && mLeftEyeClosedDuration <= MaxWinkDuration)
				{
					if (LeftEyeClosed.IsBound())
						LeftEyeClosed.Broadcast();
					mLeftEyeClosedDuration = 0;		// should ideally be mEyeClosedDuration = mEyeClosedDuration % MinEyeClosedDuration; but modulus for floats is not supported
					mLastTriggeredEyeEvent = EEyeToDetect::EYE_LEFT;
					//UE_LOG(LogTemp, Warning, TEXT("Left eye wink"));
				}
				else
				{
					//UE_LOG(LogTemp, Warning, TEXT("Left eye wink duration = %f"), mLeftEyeClosedDuration);
				}
			}
			// check for right eye wink
			if (mLastClosedEye == EEyeToDetect::EYE_RIGHT)
			{
				if (mRightEyeClosedDuration >= MinWinkDuration && mRightEyeClosedDuration <= MaxWinkDuration)
				{
					if (RightEyeClosed.IsBound())
						RightEyeClosed.Broadcast();
					mRightEyeClosedDuration = 0;
					mLastTriggeredEyeEvent = EEyeToDetect::EYE_RIGHT;
					//UE_LOG(LogTemp, Warning, TEXT("Right eye wink"));
				}
				else
				{
					//UE_LOG(LogTemp, Warning, TEXT("Right eye wink duration = %f"), mRightEyeClosedDuration);
				}
			}

			/** Both open **/
			if (mLastClosedEye != EEyeToDetect::EYE_NONE && mLastTriggeredEyeEvent != EEyeToDetect::EYE_NONE)
			{
				if (BothEyeOpened.IsBound())
					BothEyeOpened.Broadcast();
				//UE_LOG(LogTemp, Warning, TEXT("Both eyes are open"));
			}

			mLastClosedEye = EEyeToDetect::EYE_NONE;
			mLeftEyeClosedDuration = 0;
			mRightEyeClosedDuration = 0;
			mLastTriggeredEyeEvent = EEyeToDetect::EYE_NONE;

			/** Blink **/
			if (bBlinkPossible)
			{
				if (!bMarkedForReset || mBlinkErrorTimer <= BlinkErrorRange)
				{
					//mBlinkTimer += deltaSeconds; // debatable
					if (mBlinkTimer >= MinBlinkDuration && mBlinkTimer <= MaxBlinkDuration)
					{
						//UE_LOG(LogTemp, Warning, TEXT("UserBlinked"));
						bDidBlink = true;
						mBlinkCount++;
						if (UserBlinked.IsBound())
							UserBlinked.Broadcast();
					}
				}
				ResetBlinkDetection();
			}
		}
		// left eye is closed, right eye is open
		else if (bIsLeftEyeClosed && !bIsRightEyeClosed)
		{
			mLeftEyeClosedDuration += deltaSeconds;
			mRightEyeClosedDuration = 0;
			mLastClosedEye = EEyeToDetect::EYE_LEFT;

			if (bBlinkPossible)
			{
				bMarkedForReset = true;
			}
		}
		// left eye is open, right eye is closed
		else if (!bIsLeftEyeClosed && bIsRightEyeClosed)
		{
			mLeftEyeClosedDuration = 0;
			mRightEyeClosedDuration += deltaSeconds;
			mLastClosedEye = EEyeToDetect::EYE_RIGHT;

			if (bBlinkPossible)
			{
				bMarkedForReset = true;
			}
		}
		// both eyes are closed
		else
		{
			if (mLastClosedEye == EEyeToDetect::EYE_LEFT)
				mLeftEyeClosedDuration += deltaSeconds;
			else if (mLastClosedEye == EEyeToDetect::EYE_RIGHT)
				mRightEyeClosedDuration += deltaSeconds;

			bBlinkPossible = true;
			mBlinkTimer += deltaSeconds;
		}
	}
}

bool AChannelerEyeXPlayerController::IsLeftEyeClosed() const
{
	return bIsLeftEyeClosed;
}

bool AChannelerEyeXPlayerController::IsRightEyeClosed() const
{
	return bIsRightEyeClosed;
}

bool AChannelerEyeXPlayerController::DidUserBlink() const
{
	return bDidBlink;
}

int32 AChannelerEyeXPlayerController::BlinkCount() const
{
	return mBlinkCount;
}

void AChannelerEyeXPlayerController::ResetBlinkDetection()
{
	bBlinkPossible = false;
	mBlinkTimer = 0;

	mBlinkCount = 0;

	bMarkedForReset = false;
	mBlinkErrorTimer = 0;
}

void AChannelerEyeXPlayerController::SimulateLeftEyeClosed()
{
	UE_LOG(LogTemp, Warning, TEXT("SimulateLeftEyeClosed()"));
	if (IsEyeXSimulating())
		bIsLeftEyeClosed = true;
}

void AChannelerEyeXPlayerController::SimulateRightEyeClosed()
{
	if (IsEyeXSimulating())
		bIsRightEyeClosed = true;
}

void AChannelerEyeXPlayerController::SimulateLeftEyeOpen()
{
	if (IsEyeXSimulating())
		bIsLeftEyeClosed = false;
}

void AChannelerEyeXPlayerController::SimulateRightEyeOpen()
{
	if (IsEyeXSimulating())
		bIsRightEyeClosed = false;
}

bool AChannelerEyeXPlayerController::IsEyeXSimulating() const
{
	return (mGameMode != nullptr) ? mGameMode->IsEyeXSimulating() : false;
}

void AChannelerEyeXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	/* Simulation */
	InputComponent->BindAction("Simulate_LeftEyeClosed", IE_Pressed, this, &AChannelerEyeXPlayerController::SimulateLeftEyeClosed);
	InputComponent->BindAction("Simulate_RightEyeClosed", IE_Pressed, this, &AChannelerEyeXPlayerController::SimulateRightEyeClosed);

	InputComponent->BindAction("Simulate_LeftEyeClosed", IE_Released, this, &AChannelerEyeXPlayerController::SimulateLeftEyeOpen);
	InputComponent->BindAction("Simulate_RightEyeClosed", IE_Released, this, &AChannelerEyeXPlayerController::SimulateRightEyeOpen);
}

UChannelerCheatManager& AChannelerEyeXPlayerController::CheatManager()
{
	check(mCheatManager != nullptr);
	return *mCheatManager;
}

void AChannelerEyeXPlayerController::SetStoryManager(UStoryManager& storyManager)
{
	mStoryManager = &storyManager;
}

void AChannelerEyeXPlayerController::SkipCurrentStoryNode()
{
	UE_LOG(LogTemp, Warning, TEXT("UChannelerCheatManager::SkipStoryNode()"));
	if (mStoryManager != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("skipping node"));
		mStoryManager->SkipNode();
	}
}

void AChannelerEyeXPlayerController::TransitionToDialogueCam()
{
	if (mStoryManager == nullptr)
		return;

	UStoryNode* currentNode = mStoryManager->GetCurrentNode();
	if (currentNode != nullptr)
	{
		UStoryDialogueFreezeCamNode* node = Cast<UStoryDialogueFreezeCamNode>(currentNode);
		if (node != nullptr)
		{
			node->mFreezeCam->TransitionToDialogueCam();
		}
	}
}

void AChannelerEyeXPlayerController::TransferControlToPlayer()
{
	if (mStoryManager == nullptr)
		return;

	UStoryNode* currentNode = mStoryManager->GetCurrentNode();
	if (currentNode != nullptr)
	{
		UStoryDialogueFreezeCamNode* node = Cast<UStoryDialogueFreezeCamNode>(currentNode);
		if (node != nullptr)
		{
			node->mFreezeCam->EndInteraction();
		}
	}
}

void AChannelerEyeXPlayerController::JumpToStoryNode(FString nodeName)
{
	if (mStoryManager == nullptr)
		return;

	UStoryNode* nodeToJump = mStoryManager->GetNodeByName(nodeName);
	if (nodeToJump != nullptr)
		mStoryManager->SetCurrentNode(nodeToJump, true);
}

void AChannelerEyeXPlayerController::PrintScreenResolution()
{
	UGameUserSettings* gameSettings = GEngine->GetGameUserSettings();
	if (gameSettings != nullptr)
	{
		FIntPoint currentScreenRes = gameSettings->GetScreenResolution();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("%d x %d"), currentScreenRes.X, currentScreenRes.Y));
		UE_LOG(LogTemp, Warning, TEXT("%d x %d"), currentScreenRes.X, currentScreenRes.Y);
	}
}


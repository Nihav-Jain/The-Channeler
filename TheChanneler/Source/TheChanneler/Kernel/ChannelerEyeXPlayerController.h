// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EyeXPlayerController.h"
#include "ChannelerEyeXPlayerController.generated.h"

class UStoryManager;

/**
 * Enumeration for determining the device which input was last known to come from.
 */
UENUM(BlueprintType)
enum class EInputDevices : uint8
{
	ID_KBM			UMETA(DisplayName = "Keyboard/Mouse"),
	ID_Gamepad		UMETA(DisplayName = "Gamepad")
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class THECHANNELER_API AChannelerEyeXPlayerController : public AEyeXPlayerController
{
	GENERATED_BODY()
	
public:
	AChannelerEyeXPlayerController();
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	UChannelerCheatManager& CheatManager();

	void SetStoryManager(UStoryManager& storyManager);

	UFUNCTION(Exec, Category = "Storytelling")
	void SkipCurrentStoryNode();

	UFUNCTION(Exec, Category = "Storytelling")
	void TransitionToDialogueCam();

	UFUNCTION(Exec, Category = "Storytelling")
	void TransferControlToPlayer();

	UFUNCTION(Exec, BlueprintCallable, Category = "Storytelling")
	void JumpToStoryNode(FString nodeName);

	UFUNCTION(Exec, BlueprintCallable, Category = "Settings")
	void PrintScreenResolution();
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	EInputDevices GetLastKnownInputDevice() const;

	UFUNCTION(BlueprintCallable, Category = "Input")
	EInputDevices GetPreviousFramesLastKnownInputDevice() const;

private:
	class UChannelerCheatManager* mCheatManager;
	UStoryManager* mStoryManager;
	EInputDevices mLastKnownInputDevice;
	EInputDevices mPreviousFramesLastKnownInputDevice;
	bool WasKMBInputJustDetected() const;
	bool WasGamepadInputJustDetected() const;
};

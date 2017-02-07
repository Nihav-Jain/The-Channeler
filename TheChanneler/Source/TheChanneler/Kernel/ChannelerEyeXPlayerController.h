// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EyeXPlayerController.h"
#include "ChannelerEyeXPlayerController.generated.h"

class UStoryManager;
class UChannelerCheatManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXLostUserPresence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXGainedUserPresence);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXLeftEyeClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXRightEyeClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXBothEyesOpened);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXUserBlinked);

/**
*	Enum list of eyes, can be used where detection of individual eyes is required
*/
UENUM()
enum class EEyeToDetect : uint8
{
	EYE_NONE,
	EYE_LEFT,
	EYE_RIGHT,
	EYE_MAX
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

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	class UInputDeviceManager* InputDeviceManager;

	/* Blink / wink */

	/**
	 *	[Blueprint Callable] Getter for the status of LEFT EYE for the current frame
	 *	@return true if the left eye was closed in the current frame, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX")
	bool IsLeftEyeClosed() const;

	/**
	 *	[Blueprint Callable] getter for the status of RIGHT EYE for the current frame
	 *	@return true if the right eye was closed in the current frame, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX")
	bool IsRightEyeClosed() const;

	/**
	 *	[Edit Anywhere, Blueprint Read / Write] If marked true, the ChannelerCharacter will trigger the LeftEyeClosed, RightEyeClosed and BothEyesOpen events.
	 *	Default = false;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX")
	bool CheckForClosedEye;

	/**
	 *	[Edit Anywhere, Blueprint Read / Write] Time in seconds the eye should remain closed in order to trigger the LeftEyeClosed and RightEyeClosed events.
	 *	Default = 1.0 seconds
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX")
	float MinEyeClosedDuration;

	/**
	 *	[Blueprint Read only] Status of user presence in the current frame
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ChannelerEyeX")
	TEnumAsByte<EEyeXUserPresence::Type> UserPresence;

	/**
	 *	Event dispatcher - broadcasts when user presence is lost.
	 *	Blueprints can bind to event
	 */
	UPROPERTY(BlueprintAssignable, Category = "ChannelerEyeX")
	FEyeXLostUserPresence LostUserPresenceEvent;

	/**
	 *	Event dispatcher - broadcasts when user presence is regained after being lost.
	 *	Blueprints can bind to event
	 */
	UPROPERTY(BlueprintAssignable, Category = "ChannelerEyeX")
	FEyeXGainedUserPresence GainedUserPresenceEvent;

	/**
	 *	Event dispatcher - broadcasts when user the left eye is closed for more than the time specied as MinEyeClosedDuration.
	 *	Blueprints can bind to event
	 */
	UPROPERTY(BlueprintAssignable, Category = "ChannellerEyeX")
	FEyeXLeftEyeClosed LeftEyeClosed;

	/**
	 *	Event dispatcher - broadcasts when user the right eye is closed for more than the time specied as MinEyeClosedDuration.
	 *	Blueprints can bind to event
	 */
	UPROPERTY(BlueprintAssignable, Category = "ChannellerEyeX")
	FEyeXRightEyeClosed RightEyeClosed;

	/**
	 *	Event dispatcher - broadcasts when user opens both eyes after closing either of his eyes.
	 *	Blueprints can bind to event
	 */
	UPROPERTY(BlueprintAssignable, Category = "ChannellerEyeX")
	FEyeXBothEyesOpened BothEyeOpened;

	/** Blinking */

	/**
	 *	[Blueprint Callable] tells if the user blinked in the last frame (prefer to use the UserBlinked event instead)
	 *	@return true if user blinked in the last frame, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX|Blinking")
	bool DidUserBlink() const;

	/** 
	 *  [Edit Anywhere, Blueprint Read / Write] Minimum number of seconds user must have his eyes closed in order for the action to be considered as a blink.
	 *  default = 0.1 seconds;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX|Blinking", meta = (ClampMin = "0"))
	float MinBlinkDuration;

	/** 
	 *  [Edit Anywhere, Blueprint Read / Write] Max number of seconds user must have his eyes closed in order for the action to be considered as a blink.
	 *	default = 0.5 seconds;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX|Blinking", meta = (ClampMin = "0.1"))
	float MaxBlinkDuration;

	/** 
	 *  [Edit Anywhere, Blueprint Read / Write] In case of error in detecting "both eyes open" i.e. Tobii only detects 1 eye as open, number of seconds in which if both eyes open, the action will be considered a blink
	 *	default = 0.2 seconds;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX|Blinking", meta = (ClampMin = "0"))
	float BlinkErrorRange;

	/** 
	 *  [Blueprint Callable] [UNIMPLEMENTED] gets the count of number of blinks in the past few seconds if the user is continously blinking, with each blink coming in less than the value of BlinkStreakInterval
	 *	default = 0;
	 *	@return integer count
	 */
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX|Blinking")
	int32 BlinkCount() const;

	/** 
	 *  [Edit Anywhere, Blueprint Read / Write] Blink count will be incremented if the user blinks again within the number of seconds specified as this value
	 *	default = 0.5 seconds;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX|Blinking", meta = (ClampMin = "0"))
	float BlinkStreakInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX|Blinking", meta = (ClampMin = "0"))
	float MinWinkDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX|Blinking", meta = (ClampMin = "0"))
	float MaxWinkDuration;

	/**
	 *	Event dispatcher - broadcasts when user blinks.
	 *	Blueprints can bind to event
	 */
	UPROPERTY(BlueprintAssignable, Category = "ChannellerEyeX|Blinking")
	FEyeXUserBlinked UserBlinked;

	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX|Blinking")
	void BlinkWinkTick(float deltaSeconds);

	/* Extended FOV */

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Extended FOV")
	bool ExtendedFOVEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Extended FOV")
	float ExtendedFOVTurnRate;

	/* Debugging stuff */
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

	bool IsEyeXSimulating() const;

protected:
	virtual void SetupInputComponent() override;

private:
	void ResetBlinkDetection();
	
	UChannelerCheatManager* mCheatManager;
	UStoryManager* mStoryManager;

	class IEyeXPlugin* mEyeX;
	bool bIsLeftEyeClosed;
	bool bIsRightEyeClosed;

	float mLeftEyeClosedDuration;
	float mRightEyeClosedDuration;

	EEyeToDetect mLastClosedEye;
	EEyeToDetect mLastTriggeredEyeEvent;

	bool bDidBlink;
	int32 mBlinkCount;

	float mBlinkTimer;
	bool bBlinkPossible;
	bool bMarkedForReset;
	float mBlinkErrorTimer;

	/** EyeX Simulation */

	void SimulateLeftEyeClosed();
	void SimulateRightEyeClosed();
	void SimulateLeftEyeOpen();
	void SimulateRightEyeOpen();

	class ATheChannelerGameMode* mGameMode;
};

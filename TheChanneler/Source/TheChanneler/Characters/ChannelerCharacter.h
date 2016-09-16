// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VisualLogger.h"
#include "FP_FirstPerson/FP_FirstPersonCharacter.h"
#include "IEyeXPlugin.h"
#include "../Storytelling/StoryManager.h"
#include "ChannelerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXLostUserPresence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXGainedUserPresence);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXLeftEyeClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXRightEyeClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXBothEyesOpened);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEyeXUserBlinked);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSkipLevel, FString, SkipName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChannelerStoryParsed, UStoryManager*, ActiveStoryManager);

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

USTRUCT(BlueprintType)
struct FExtendedFOVMargin
{
	GENERATED_BODY()
public:
	FExtendedFOVMargin();
	FExtendedFOVMargin(float left, float right, float top, float bottom);

	/* Fraction of screen width from the left for extended FOV (0.0 <= left < 0.5) */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, ClampMax = 0.49f))
	float Left;

	/* Fraction of screen width from the right for extended FOV (0.0 <= right < 0.5) */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, ClampMax = 0.49f))
	float Right;

	/* Fraction of screen height from the top for extended FOV (0.0 <= top < 0.5) */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, ClampMax = 0.49f))
	float Top;

	/* Fraction of screen height from the bottom for extended FOV (0.0 <= bottom < 0.5) */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, ClampMax = 0.49f))
	float Bottom;
};

/**
 *	Custom First-Person Character for TheChanneler
 */
UCLASS(BlueprintType, Blueprintable)
class THECHANNELER_API AChannelerCharacter : public AFP_FirstPersonCharacter
{
	GENERATED_BODY()

#if ENABLE_VISUAL_LOG
	/** Appends information about this actor to the visual logger */
	virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif

public:
	AChannelerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Re-enables the character movement */
	UFUNCTION(BlueprintCallable, Category="Channeler")
	void EnableMovement();

	/** Disables the character movement */
	UFUNCTION(BlueprintCallable, Category = "Channeler")
	void DisableMovement();

	UFUNCTION(BlueprintCallable, Category = "Channeler")
	void EnableLook();

	UFUNCTION(BlueprintCallable, Category = "Channeler")
	void DisableLook();

	UFUNCTION(BlueprintCallable, Category = "Channeler")
	bool IsMovementEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Channeler")
	bool IsLookEnabled() const;

	/**
	*	[Blueprint Callable] Getter for the status of LEFT EYE for the current frame
	*	@return true if the left eye was closed in the current frame, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX")
		bool IsLeftEyeClosed() const;

	/**
	*	[Blueprint Callable] Changes the KeyBoard sensitivity
	*/
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX")
		void MouseSensitivity(float Sensitivity);

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ChannelerEyeX")
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

	/**
	 *	Event dispatcher - broadcasts when user the eagle eye effect should be disabled after it times out.
	 *	Blueprints can bind to event
	 */
	//UPROPERTY(BlueprintAssignable, Category = "Eagle Eye")
	//FForceDeactivateEagleEye(ForceDeactivateEagleEyeEvent);

	/** Maintains a reference to the current camera being used in the game for the world viewport. Comes in handy for camera realted operations when FreezeCam is active */
	UPROPERTY(BlueprintReadOnly, Category = "ChannelerEyeX")
	UCameraComponent* ActiveCameraComponent;


	/** Blinking */

	/**
	 *	[Blueprint Callable] tells if the user blinked in the last frame (prefer to use the UserBlinked event instead)
	 *	@return true if user blinked in the last frame, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX | Blinking")
	bool DidUserBlink() const;

	/** [Edit Anywhere, Blueprint Read / Write] Minimum number of seconds user must have his eyes closed in order for the action to be considered as a blink. 
	 *	default = 0.1 seconds; 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX | Blinking", meta = (ClampMin = "0"))
	float MinBlinkDuration;

	/** [Edit Anywhere, Blueprint Read / Write] Max number of seconds user must have his eyes closed in order for the action to be considered as a blink.
	 *	default = 0.5 seconds;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX | Blinking", meta = (ClampMin = "0.1"))
	float MaxBlinkDuration;

	/** [Edit Anywhere, Blueprint Read / Write] In case of error in detecting "both eyes open" i.e. Tobii only detects 1 eye as open, number of seconds in which if both eyes open, the action will be considered a blink
	 *	default = 0.2 seconds;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX | Blinking", meta = (ClampMin = "0"))
	float BlinkErrorRange;

	/** [Edit Anywhere, Blueprint Read / Write] The maximum duration of the most recent eagle eye toggle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eagle Eye")
	float MaxEagleEyeDuration;

	/** [Blueprint Callable] [UNIMPLEMENTED] gets the count of number of blinks in the past few seconds if the user is continously blinking, with each blink coming in less than the value of BlinkStreakInterval
	 *	default = 0;
	 *	@return integer count
	 */
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX | Blinking")
	int32 BlinkCount() const;

	/** [Edit Anywhere, Blueprint Read / Write] Blink count will be incremented if the user blinks again within the number of seconds specified as this value
	 *	default = 0.5 seconds;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX | Blinking", meta = (ClampMin = "0"))
	float BlinkStreakInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX | Blinking", meta = (ClampMin = "0"))
	float MinWinkDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChannelerEyeX | Blinking", meta = (ClampMin = "0"))
	float MaxWinkDuration;

	UPROPERTY(BlueprintReadWrite, Category = "Eagle Eye")
	bool bShouldEagleEyeBeDeactivated;

	/**
	 *	Event dispatcher - broadcasts when user blinks.
	 *	Blueprints can bind to event
	 */
	UPROPERTY(BlueprintAssignable, Category = "ChannellerEyeX | Blinking")
	FEyeXUserBlinked UserBlinked;

	UPROPERTY(EditDefaultsOnly, Category = "Channeler")
	FString SkipInputBindingPrefix;

	UPROPERTY(BlueprintAssignable, Category = "Channeler")
	FSkipLevel SkipLevel;

	UPROPERTY(BlueprintAssignable, Category = "Channeler")
	FChannelerStoryParsed StoryParsed;

	void CallStoryParsed(class UStoryManager* manager);

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	bool IsEagleEyeEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	void EnableEagleEye();

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	void DisableEagleEye();

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	bool IsEagleEyeActive() const;

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	bool IsRightEagleEyeActive() const;

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	bool IsLeftEagleEyeActive() const;

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	void ActivateRightEagleEye();

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	void ActivateLeftEagleEye();

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	void DeactivateRightEagleEye();

	UFUNCTION(BlueprintCallable, Category = "Eagle Eye")
	void DeactivateLeftEagleEye();

	UFUNCTION(BlueprintCallable, Category = "Channeler")
	bool IsInPuzzle() const;

	UFUNCTION(BlueprintCallable, Category = "Channeler")
	void SetIsInPuzzle(bool isInPuzzle);

	UFUNCTION(BlueprintImplementableEvent, Category = "Eagle Eye")
	void TurnOffEagleEye();

	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX | Blinking")
	void BlinkWinkTick(float deltaSeconds);


	/** Extended FOV */
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Extended FOV")
	bool ExtendedFOVEnabled;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Extended FOV")
	FExtendedFOVMargin ExtendedFOVMargin;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Extended FOV")
	float ExtendedFOVTurnRate;

	/* If true, rate of turn will interpolate between 0 to ExtendedFOVTurnRate from margin start to edge of screen */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Extended FOV")
	bool GradientSpeed;

	/* If true, both mouse and fov inputs wil be considered in the same frame */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Extended FOV")
	bool MouseVsFov;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void MoveForward(float Val) override;
	virtual void MoveRight(float Val) override;
	virtual void AddControllerYawInput(float Val) override;
	virtual void AddControllerPitchInput(float Val) override;
	virtual void TurnAtRate(float Rate) override;
	virtual void LookUpAtRate(float Rate) override;

private:
	void SkipLevelAction();
	void ResetBlinkDetection();

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

	bool bMovementEnabled;
	bool bLookEnabled;
	bool bIsEagleEyeEnabled;
	bool bIsRightEagleEyeActive;
	bool bIsLeftEagleEyeActive;
	bool bIsInPuzzle;

	float EagleEyeDurationInSeconds;

	float Sensitivity;

	TMap<FName, FKey> mKeyMappings;

	/** Extended FOV */
	
	FIntPoint mViewportCenter;
	FIntPoint mViewportSize;
	FVector4 mFOVMargin;
	bool mMouseWasMoved;

	void ExtendedFOV();
};

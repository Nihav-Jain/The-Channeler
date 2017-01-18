// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VisualLogger.h"
#include "FP_FirstPerson/FP_FirstPersonCharacter.h"
#include "IEyeXPlugin.h"
#include "../Storytelling/StoryManager.h"
#include "ChannelerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSkipLevel, FString, SkipName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChannelerStoryParsed, UStoryManager*, ActiveStoryManager);

UENUM(BlueprintType)
enum class EExtendedFOVMode : uint8
{
	InfiniteScreen		UMETA(DisplayName = "Infinite Screen"),
	ExtendedScreen		UMETA(DisplayName = "Extended Screen"),
	EllipticalExtendedScreen	UMETA(DisplayName = "Elliptical Extended Screen")
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

	UFUNCTION(BlueprintCallable, Category = "Channeler")
	FRotator GetCharacterViewRotation() const;

	/**
	*	[Blueprint Callable] Changes the Mouse and Controller sensitivity
	*/
	UFUNCTION(BlueprintCallable, Category = "ChannelerEyeX")
	void MouseSensitivity(float Sensitivity);

	/**
	 *	Event dispatcher - broadcasts when user the eagle eye effect should be disabled after it times out.
	 *	Blueprints can bind to event
	 */
	//UPROPERTY(BlueprintAssignable, Category = "Eagle Eye")
	//FForceDeactivateEagleEye(ForceDeactivateEagleEyeEvent);

	/** Maintains a reference to the current camera being used in the game for the world viewport. Comes in handy for camera realted operations when FreezeCam is active */
	UPROPERTY(BlueprintReadOnly, Category = "ChannelerEyeX")
	UCameraComponent* ActiveCameraComponent;

	/** [Edit Anywhere, Blueprint Read / Write] The maximum duration of the most recent eagle eye toggle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eagle Eye")
	float MaxEagleEyeDuration;

	UPROPERTY(BlueprintReadWrite, Category = "Eagle Eye")
	bool bShouldEagleEyeBeDeactivated;

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

	/** Extended FOV */

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Extended FOV")
	EExtendedFOVMode ExtendedFOVMode;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Extended FOV")
	FExtendedFOVMargin ExtendedFOVMargin;

	/* If true, rate of turn will interpolate between 0 to ExtendedFOVTurnRate from margin start to edge of screen */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Extended FOV")
	bool GradientSpeed;

	/* If true, both mouse and fov inputs wil be considered in the same frame */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Extended FOV")
	bool MouseVsFov;

	UPROPERTY(EditAnywhere, Category = "Extended FOV")
	bool Easing;

	UPROPERTY(EditAnywhere, Category = "Extended FOV", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float EasingResponsiveness;

	UPROPERTY(EditAnywhere, Category = "Extended FOV", meta = (ClampMin = 1, ClampMax = 89))
	FVector2D ExtendedScreenMaxAngle;

	UPROPERTY(EditAnywhere, Category = "Extended FOV", meta = (ClampMin = 1, ClampMax = 89))
	FVector2D ExtendedScreenFilterAngle;

	/* Ellipse axes, centered at the center of the screen. Inside of ellipse is unaffected by FOV. 
	MajorAxisRatio = (MajorAxisLength / ScreenWidth); MinorAxisRatio = (MinorAxisLength / ScreenHeight);  
	*/
	UPROPERTY(EditAnywhere, Category = "Extended FOV", meta = (ClampMin = 0.01f, ClampMax = 1.0f))
	FVector2D NoFOVEllipseAxesRatio;

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

	UFUNCTION(BlueprintCallable, Category = "Screen Resolution")
	void UpdateScreenResolutionRelatedProperties(int32 newWidth, int32 newHeight);

private:
	void SkipLevelAction();

	class IEyeXPlugin* mEyeX;

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
	
	class AGhostCameraActor* mGhostCamActor;
	FIntPoint mViewportCenter;
	FIntPoint mViewportSize;
	FVector4 mFOVMargin;
	bool mMouseWasMoved;
	FRotator mFOVCameraRotation;

	FVector2D mFOVEllipseAxes;
	FVector2D mFOVEllipseAxesSquared;
	FVector2D mFOVOuterEllipseAxes;
	FVector2D mFOVOuterEllipseAxesSquared;

	void ExtendedFOV();
	void InfiniteScreenFOV(const FVector2D& relativeGazePoint, const FVector2D& speedInterpolation, float turnRate);
	void ExtendedScreenFOV(const FVector2D& relativeGazePoint, const FVector2D& speedInterpolation);
	void EllipticalExtendedScreenFOV(const FVector2D& relativeGazePoint, float turnRate);

};

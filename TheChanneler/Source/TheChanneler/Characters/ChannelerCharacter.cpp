// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerCharacter.h"
#include "../Kernel/ChannelerCheatManager.h"
#include "../Utils/ChannelerUtils.h"
#include "../Storytelling/StoryManager.h"
#include "../TheChannelerHUD.h"
#include "IEyeXPlugin.h"

FExtendedFOVMargin::FExtendedFOVMargin() :
	Left(0.2f), Right(0.2f), Top(0.2f), Bottom(0.2f)
{}

FExtendedFOVMargin::FExtendedFOVMargin(float left, float right, float top, float bottom) :
	Left(left), Right(right), Top(top), Bottom(bottom)
{}

#if ENABLE_VISUAL_LOG
void AChannelerCharacter::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	Super::GrabDebugSnapshot(Snapshot);

	FVisualLogStatusCategory MyCategory;
	MyCategory.Category = TEXT("Heat Map");
	MyCategory.Add(TEXT("Test Value"), TEXT("10"));
	MyCategory.Add(TEXT("String"), GetDebugName(this));
	/*
	FVisualLogStatusCategory MySubCategory;
	MySubCategory.Category = TEXT("My Sub Category");
	MySubCategory.Add(TEXT("Test Value 2"), TEXT("13"));
	MyCategory.AddChild(MySubCategory);*/

	Snapshot->Status.Add(MyCategory);
}
#endif

AChannelerCharacter::AChannelerCharacter() :
	CheckForClosedEye(false), MinEyeClosedDuration(1.0f),
	bIsLeftEyeClosed(false), bIsRightEyeClosed(false),
	mEyeX(nullptr), UserPresence(EEyeXUserPresence::Unknown),
	mLeftEyeClosedDuration(0.0f), mRightEyeClosedDuration(0.0f),
	mLastClosedEye(EEyeToDetect::EYE_NONE), mLastTriggeredEyeEvent(EEyeToDetect::EYE_NONE),
	ActiveCameraComponent(nullptr), EagleEyeDurationInSeconds(0.0f),
	bDidBlink(false), MinBlinkDuration(0.1f), MaxBlinkDuration(0.5f), mBlinkCount(0), BlinkStreakInterval(0.5),
	mBlinkTimer(0), bBlinkPossible(false), bMarkedForReset(false), BlinkErrorRange(0.2f), mBlinkErrorTimer(0),
	MinWinkDuration(0.25f), MaxWinkDuration(0.5f), bShouldEagleEyeBeDeactivated(false),
	bMovementEnabled(true), bLookEnabled(true), Sensitivity(1.0f), bIsInPuzzle(false),
	bIsEagleEyeEnabled(false), bIsRightEagleEyeActive(false), bIsLeftEagleEyeActive(false),
	SkipInputBindingPrefix("Skip_"), mKeyMappings(), SkipLevel(),
	ExtendedFOVMargin(), ExtendedFOVEnabled(true), ExtendedFOVTurnRate(1.0f), GradientSpeed(false),
	mViewportCenter(1920/2, 1080/2), mViewportSize(1920, 1080), MouseVsFov(true), mMouseWasMoved(false), 
	Easing(false), EasingResponsiveness(0.25f)
{}

void AChannelerCharacter::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("AChannelerCharacter::BeginPlay()"));
	Super::BeginPlay();

	mEyeX = &IEyeXPlugin::Get();
	UserPresence = mEyeX->GetUserPresence();
	ActiveCameraComponent = GetFirstPersonCameraComponent();

	DisableEagleEye();

	UChannelerUtils::SetChanneler(this);

	if (GEngine != nullptr)
	{
		mViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
		mViewportCenter = FIntPoint(mViewportSize.X / 2, mViewportSize.Y / 2);

		UE_LOG(LogTemp, Warning, TEXT("Viewport size = %d %d"), mViewportSize.X, mViewportSize.Y);
	}

	mFOVMargin = FVector4(
		mViewportSize.X * ExtendedFOVMargin.Left, 
		mViewportSize.Y * ExtendedFOVMargin.Top, 
		mViewportSize.X * ExtendedFOVMargin.Right,
		mViewportSize.Y * ExtendedFOVMargin.Bottom
	);
}

void AChannelerCharacter::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	ExtendedFOV();

	// Get the duration that the eagle eye has been active
	if (IsEagleEyeActive())
	{
		EagleEyeDurationInSeconds += deltaSeconds;
		if (EagleEyeDurationInSeconds >= MaxEagleEyeDuration)
		{
			EagleEyeDurationInSeconds = 0.0f;
			DeactivateLeftEagleEye();
			DeactivateRightEagleEye();
			bShouldEagleEyeBeDeactivated = true;
		}
	}
	else
	{
		EagleEyeDurationInSeconds = 0.0f;
	}

	BlinkWinkTick(deltaSeconds);
}

void AChannelerCharacter::EnableMovement()
{
	//GetCharacterMovement()->ResetMoveState();
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bMovementEnabled = true;
}

void AChannelerCharacter::DisableMovement()
{
	//GetCharacterMovement()->DisableMovement();
	bMovementEnabled = false;
}

void AChannelerCharacter::EnableLook()
{
	bLookEnabled = true;
}

void AChannelerCharacter::DisableLook()
{
	bLookEnabled = false;
}

bool AChannelerCharacter::IsMovementEnabled() const
{
	return bMovementEnabled;
}

bool AChannelerCharacter::IsLookEnabled() const
{
	return bLookEnabled;
}

bool AChannelerCharacter::IsLeftEyeClosed() const
{
	return bIsLeftEyeClosed;
}

void AChannelerCharacter::MouseSensitivity(float sensitivity)
{
	Sensitivity = sensitivity;
	//BaseTurnRate = Sensitivity;
	//BaseLookUpRate = Sensitivity;
	//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, FString::Printf(TEXT("BaseLookUpRate: %f"), BaseLookUpRate));

}

bool AChannelerCharacter::IsRightEyeClosed() const
{
	return bIsRightEyeClosed;
}

bool AChannelerCharacter::DidUserBlink() const
{
	return bDidBlink;
}

int32 AChannelerCharacter::BlinkCount() const
{
	return mBlinkCount;
}

void AChannelerCharacter::ResetBlinkDetection()
{
	bBlinkPossible = false;
	mBlinkTimer = 0;

	mBlinkCount = 0;

	bMarkedForReset = false;
	mBlinkErrorTimer = 0;
}

void AChannelerCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	//InputComponent->BindAction("Fire", IE_Pressed, this, &AFP_FirstPersonCharacter::OnFire);
	TryEnableTouchscreenMovement(InputComponent);

	// Bind movement events
	InputComponent->BindAxis("MoveForward", this, &AChannelerCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AChannelerCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &AChannelerCharacter::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AChannelerCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &AChannelerCharacter::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AChannelerCharacter::LookUpAtRate);

#if CHANNELER_SHIP_TEST

	// Bind jump events
	//InputComponent->BindAction("Jump", IE_Pressed, this, &AChannelerCharacter::Jump);
	//InputComponent->BindAction("Jump", IE_Released, this, &AChannelerCharacter::StopJumping);

	const UInputSettings* inputSettings = GetDefault<UInputSettings>();
	if (inputSettings == nullptr)
		return;
	
	int32 i;
	for (i = 0; i < inputSettings->ActionMappings.Num(); i++)
	{
		const FString& actionName = inputSettings->ActionMappings[i].ActionName.ToString();
		if (actionName.Find(SkipInputBindingPrefix) == 0)
		{
			mKeyMappings.Add(inputSettings->ActionMappings[i].ActionName, inputSettings->ActionMappings[i].Key);
			InputComponent->BindAction(inputSettings->ActionMappings[i].ActionName, IE_Pressed, this, &AChannelerCharacter::SkipLevelAction);
			UE_LOG(LogTemp, Warning, TEXT("Registered %s as a SkipLevelAction"), *actionName);
		}
	}

#endif
}

void AChannelerCharacter::Jump()
{
	if (bMovementEnabled)
		Super::Jump();
}

void AChannelerCharacter::StopJumping()
{
	if (bMovementEnabled)
		Super::StopJumping();
}

void AChannelerCharacter::MoveForward(float Val)
{
	if (bMovementEnabled)
		Super::MoveForward(Val);
}

void AChannelerCharacter::MoveRight(float Val)
{
	if (bMovementEnabled)
		Super::MoveRight(Val);
}

void AChannelerCharacter::AddControllerYawInput(float Val)
{
	if (bLookEnabled)
	{
		mMouseWasMoved = (Val != 0.0f);
		Super::AddControllerYawInput(Val * Sensitivity);
	}
}

void AChannelerCharacter::TurnAtRate(float Rate)
{
	if (bLookEnabled)
	{
		mMouseWasMoved = (Rate != 0.0f);
		Super::TurnAtRate(Rate);
	}
}

void AChannelerCharacter::AddControllerPitchInput(float Val)
{
	if (bLookEnabled)
	{
		mMouseWasMoved = (Val != 0.0f);
		Super::AddControllerPitchInput(Val * Sensitivity);
	}
}

void AChannelerCharacter::LookUpAtRate(float Rate)
{
	if (bLookEnabled)
	{
		mMouseWasMoved = (Rate != 0.0f);
		Super::LookUpAtRate(Rate);
	}
}

void AChannelerCharacter::SkipLevelAction()
{
	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (controller == nullptr)
		return;

	for (auto& pair : mKeyMappings)
	{
		if (controller->WasInputKeyJustPressed(pair.Value))
		{
			UE_LOG(LogTemp, Warning, TEXT("Skip Action performed = %s"), *(pair.Key.ToString()));
			if (SkipLevel.IsBound())
				SkipLevel.Broadcast(pair.Key.ToString());
			break;
		}
	}
}

void AChannelerCharacter::CallStoryParsed(UStoryManager* manager)
{
	if (StoryParsed.IsBound())
		StoryParsed.Broadcast(manager);
}

bool AChannelerCharacter::IsEagleEyeEnabled() const
{
	return bIsEagleEyeEnabled;
}

void AChannelerCharacter::EnableEagleEye()
{
	bIsEagleEyeEnabled = true;
}

void AChannelerCharacter::DisableEagleEye()
{
	DeactivateRightEagleEye();
	DeactivateLeftEagleEye();
	bIsEagleEyeEnabled = false;
}

bool AChannelerCharacter::IsEagleEyeActive() const
{
	return (IsRightEagleEyeActive() || IsLeftEagleEyeActive());
}

bool AChannelerCharacter::IsRightEagleEyeActive() const
{
	return bIsRightEagleEyeActive;
}

bool AChannelerCharacter::IsLeftEagleEyeActive() const
{
	return bIsLeftEagleEyeActive;
}

void AChannelerCharacter::ActivateRightEagleEye()
{
	// Early out so we dont reset timers
	if (bIsRightEagleEyeActive || !bIsEagleEyeEnabled)
		return;

	ATheChannelerHUD* ChannelerHud = Cast<ATheChannelerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	if (ChannelerHud != nullptr)
	{
		ChannelerHud->TurnOnGaze();
	}

	EagleEyeDurationInSeconds = 0.0f;
	bIsRightEagleEyeActive = true;
}

void AChannelerCharacter::ActivateLeftEagleEye()
{
	// Early out so we dont reset timers
	if (bIsLeftEagleEyeActive || !bIsEagleEyeEnabled)
		return;

	ATheChannelerHUD* ChannelerHud = Cast<ATheChannelerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	if (ChannelerHud != nullptr)
	{
		ChannelerHud->TurnOnGaze();
	}

	EagleEyeDurationInSeconds = 0.0f;
	bIsLeftEagleEyeActive = true;
}

void AChannelerCharacter::DeactivateRightEagleEye()
{
	if (!IsInPuzzle())
	{
		ATheChannelerHUD* ChannelerHud = Cast<ATheChannelerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		if (ChannelerHud != nullptr)
		{
			ChannelerHud->TurnOffGaze();
		}
	}

	bIsRightEagleEyeActive = false;
	TurnOffEagleEye();
}

void AChannelerCharacter::DeactivateLeftEagleEye()
{
	if (!IsInPuzzle())
	{
		ATheChannelerHUD* ChannelerHud = Cast<ATheChannelerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		if (ChannelerHud != nullptr)
		{
			ChannelerHud->TurnOffGaze();
		}
	}

	bIsLeftEagleEyeActive = false;
	TurnOffEagleEye();
}

void AChannelerCharacter::BlinkWinkTick(float deltaSeconds)
{
	// check for user presence; if it has changed since last frame, trigger appropriate event
	TEnumAsByte<EEyeXUserPresence::Type> currentUserPresence = mEyeX->GetUserPresence();
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

	bDidBlink = false;

	if (CheckForClosedEye && currentUserPresence == EEyeXUserPresence::Present)
	{
		FEyeXEyePosition currentEyePosition = mEyeX->GetEyePosition();

		bIsLeftEyeClosed = !currentEyePosition.bIsLeftEyeValid;
		bIsRightEyeClosed = !currentEyePosition.bIsRightEyeValid;

		if (bMarkedForReset)
			mBlinkErrorTimer += deltaSeconds;

		// both eyes are open
		if (currentEyePosition.bIsLeftEyeValid && currentEyePosition.bIsRightEyeValid)
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
		else if (!currentEyePosition.bIsLeftEyeValid && currentEyePosition.bIsRightEyeValid)
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
		else if (currentEyePosition.bIsLeftEyeValid && !currentEyePosition.bIsRightEyeValid)
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

bool AChannelerCharacter::IsInPuzzle() const
{
	return bIsInPuzzle;
}

void AChannelerCharacter::SetIsInPuzzle(bool isInPuzzle)
{
	bIsInPuzzle = isInPuzzle;
}

void AChannelerCharacter::ExtendedFOV()
{
	if (!(bLookEnabled && ExtendedFOVEnabled))
		return;

	if (!MouseVsFov && mMouseWasMoved)
		return;

	//TEyeXMaybeValue<FEyeXScreenBounds> screenbounds = mEyeX->GetScreenBounds();
	FEyeXGazePoint gazePoint = mEyeX->GetGazePoint(EEyeXGazePointDataMode::LightlyFiltered);
	if (gazePoint.bHasValue)
	{
		if ((gazePoint.Value.X < mFOVMargin.X)
			|| (gazePoint.Value.X > (mViewportSize.X - mFOVMargin.Z))
			|| (gazePoint.Value.Y < mFOVMargin.Y)
			|| (gazePoint.Value.Y > (mViewportSize.Y - mFOVMargin.W))
			)
		{
			// Check if gaze point is outside the screen.
			if ((gazePoint.Value.X < 0)
				|| (gazePoint.Value.X > mViewportSize.X)
				|| (gazePoint.Value.Y < 0)
				|| (gazePoint.Value.Y > mViewportSize.Y)
				)
			{
				return;
			}

			//UE_LOG(LogTemp, Warning, TEXT("Gaze Point = %f %f"), gazePoint.Value.X, gazePoint.Value.Y);
			FVector2D relativeGazePoint = FVector2D(gazePoint.Value.X - mViewportCenter.X, gazePoint.Value.Y - mViewportCenter.Y);
			relativeGazePoint.Normalize();

			FVector2D speedInterpolation = FVector2D(1.0f, 1.0f);
			if (GradientSpeed)
			{
				if (gazePoint.Value.X < mFOVMargin.X)
					speedInterpolation.X = (mFOVMargin.X - gazePoint.Value.X) / mFOVMargin.X;
				else if (gazePoint.Value.X > (mViewportSize.X - mFOVMargin.Z))
					speedInterpolation.X = (mFOVMargin.Z - (mViewportSize.X - gazePoint.Value.X)) / mFOVMargin.Z;
				if (gazePoint.Value.Y < mFOVMargin.Y)
					speedInterpolation.Y = (mFOVMargin.Y - gazePoint.Value.Y) / mFOVMargin.Y;
				else if (gazePoint.Value.Y > (mViewportSize.Y - mFOVMargin.W))
					speedInterpolation.Y = (mFOVMargin.W - (mViewportSize.Y - gazePoint.Value.Y)) / mFOVMargin.W;
			}

			FVector2D fovSpeed = FVector2D(relativeGazePoint.X * ExtendedFOVTurnRate * speedInterpolation.X,
										relativeGazePoint.Y * ExtendedFOVTurnRate * speedInterpolation.Y);

			APlayerController* const playerController = GetWorld()->GetFirstPlayerController();
			
			float deltaYaw = fovSpeed.X * playerController->InputYawScale;
			float deltaPitch = fovSpeed.Y * playerController->InputPitchScale;

			AddControllerYawInput(fovSpeed.X);
			AddControllerPitchInput(fovSpeed.Y);
		}
	}

	mMouseWasMoved = false;
}

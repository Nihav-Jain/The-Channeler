// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerCharacter.h"
#include "../Kernel/ChannelerCheatManager.h"
#include "../Utils/ChannelerUtils.h"
#include "../Storytelling/StoryManager.h"
#include "../TheChannelerHUD.h"
#include "../TheChannelerGameMode.h"
#include "GhostCameraActor.h"
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
	mEyeX(nullptr),
	ActiveCameraComponent(nullptr), EagleEyeDurationInSeconds(0.0f),
	bShouldEagleEyeBeDeactivated(false),
	bMovementEnabled(true), bLookEnabled(true), Sensitivity(1.0f), bIsInPuzzle(false),
	bIsEagleEyeEnabled(false), bIsRightEagleEyeActive(false), bIsLeftEagleEyeActive(false),
	SkipInputBindingPrefix("Skip_"), mKeyMappings(), SkipLevel(),
	ExtendedFOVMargin(), ExtendedFOVMode(EExtendedFOVMode::EllipticalExtendedScreen), GradientSpeed(true),
	mViewportCenter(1920 / 2, 1080 / 2), mViewportSize(1920, 1080), MouseVsFov(true), mMouseWasMoved(false), ExtendedScreenMaxAngle(30, 30),
	Easing(true), EasingResponsiveness(0.25f), mFOVCameraRotation(0, 0, 0), ExtendedScreenFilterAngle(1.0f, 1.0f),
	mGhostCamActor(nullptr), mFOVEllipseAxes(0, 0), NoFOVEllipseAxesRatio(0.1f, 0.1f)
{}

void AChannelerCharacter::BeginPlay()
{
	Super::BeginPlay();

	mEyeX = &IEyeXPlugin::Get();
	ActiveCameraComponent = GetFirstPersonCameraComponent();

	DisableEagleEye();

	UChannelerUtils::SetChanneler(this);

	if (GEngine != nullptr)
	{
		mViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
		mViewportCenter = FIntPoint(mViewportSize.X / 2, mViewportSize.Y / 2);

		//UE_LOG(LogTemp, Warning, TEXT("Viewport size = %d %d"), mViewportSize.X, mViewportSize.Y);
	}

	mFOVMargin = FVector4(
		mViewportSize.X * ExtendedFOVMargin.Left, 
		mViewportSize.Y * ExtendedFOVMargin.Top, 
		mViewportSize.X * ExtendedFOVMargin.Right,
		mViewportSize.Y * ExtendedFOVMargin.Bottom
	);

	mFOVEllipseAxes = FVector2D(NoFOVEllipseAxesRatio.X * mViewportSize.X / 2, NoFOVEllipseAxesRatio.Y * mViewportSize.Y / 2);
	mFOVEllipseAxesSquared = FVector2D(FMath::Square(mFOVEllipseAxes.X), FMath::Square(mFOVEllipseAxes.Y));
	mFOVOuterEllipseAxes = FVector2D(mViewportSize.X / FMath::Sqrt(2), mViewportSize.Y / FMath::Sqrt(2));
	mFOVOuterEllipseAxesSquared = FVector2D(FMath::Square(mFOVOuterEllipseAxes.X), FMath::Square(mFOVOuterEllipseAxes.Y));
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

FRotator AChannelerCharacter::GetCharacterViewRotation() const
{
	return GetViewRotation();
}

void AChannelerCharacter::MouseSensitivity(float sensitivity)
{
	Sensitivity = sensitivity;
	//BaseTurnRate = Sensitivity;
	//BaseLookUpRate = Sensitivity;
	//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, FString::Printf(TEXT("BaseLookUpRate: %f"), BaseLookUpRate));

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
			//UE_LOG(LogTemp, Warning, TEXT("Registered %s as a SkipLevelAction"), *actionName);
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
	// Somewhere the Tobii mouse emulation is being set. 
	// Until that is found and shut down, this check will prevent extended fov from working with mouse emulation mode.
	EEyeXDeviceStatus::Type deviceStatus = mEyeX->GetEyeTrackingDeviceStatus();
	if (deviceStatus != EEyeXDeviceStatus::Tracking)
		return;

	UWorld* world = GetWorld();
	if (world == nullptr)
		return;
	APlayerController* controller = world->GetFirstPlayerController();
	if (controller == nullptr)
		return;
	AChannelerEyeXPlayerController* channelerEyeXController = Cast<AChannelerEyeXPlayerController>(controller);
	if (channelerEyeXController == nullptr)
		return;
	if (!(bLookEnabled && channelerEyeXController->ExtendedFOVEnabled && !channelerEyeXController->IsEyeXSimulating()))
		return;

	if (!MouseVsFov && mMouseWasMoved)
		return;

	FEyeXGazePoint gazePoint = mEyeX->GetGazePoint(EEyeXGazePointDataMode::LightlyFiltered);
	if (gazePoint.bHasValue)
	{
		FVector2D relativeGazePoint = FVector2D(gazePoint.Value.X - mViewportCenter.X, gazePoint.Value.Y - mViewportCenter.Y);
		if (ExtendedFOVMode == EExtendedFOVMode::EllipticalExtendedScreen)
		{
			EllipticalExtendedScreenFOV(FVector2D(relativeGazePoint), channelerEyeXController->ExtendedFOVTurnRate);
			return;
		}

		if ((gazePoint.Value.X < mFOVMargin.X)
			|| (gazePoint.Value.X > (mViewportSize.X - mFOVMargin.Z))
			|| (gazePoint.Value.Y < mFOVMargin.Y)
			|| (gazePoint.Value.Y > (mViewportSize.Y - mFOVMargin.W))
			)
		{
			// Check if gaze point is outside the screen.
			if (ExtendedFOVMode == EExtendedFOVMode::ExtendedScreen)
			{
				FVector2D speedInterpolation = FVector2D(0, 0);
				if (gazePoint.Value.X < 0)
					speedInterpolation.X = -1;
				else if (gazePoint.Value.X > mViewportSize.X)
					speedInterpolation.X = 1;
				if (gazePoint.Value.Y < 0)
					speedInterpolation.Y = 1;
				else if (gazePoint.Value.Y > mViewportSize.Y)
					speedInterpolation.Y = -1;
				ExtendedScreenFOV(FVector2D(0, 0), speedInterpolation);
			}
			else if ((gazePoint.Value.X < 0)
				|| (gazePoint.Value.X > mViewportSize.X)
				|| (gazePoint.Value.Y < 0)
				|| (gazePoint.Value.Y > mViewportSize.Y)
				)
			{
				return;
			}

			relativeGazePoint.Normalize();

			FVector2D speedInterpolation = FVector2D(0.0f, 0.0f);

			if (GradientSpeed)
			{
				if (gazePoint.Value.X < mFOVMargin.X)
				{
					speedInterpolation.X = (mFOVMargin.X - gazePoint.Value.X) / mFOVMargin.X;
					if (ExtendedFOVMode == EExtendedFOVMode::ExtendedScreen)
						speedInterpolation.X = -speedInterpolation.X;
				}
				else if (gazePoint.Value.X > (mViewportSize.X - mFOVMargin.Z))
				{
					speedInterpolation.X = (mFOVMargin.Z - (mViewportSize.X - gazePoint.Value.X)) / mFOVMargin.Z;
				}
				if (gazePoint.Value.Y < mFOVMargin.Y)
				{
					speedInterpolation.Y = (mFOVMargin.Y - gazePoint.Value.Y) / mFOVMargin.Y;
				}
				else if (gazePoint.Value.Y > (mViewportSize.Y - mFOVMargin.W))
				{
					speedInterpolation.Y = (mFOVMargin.W - (mViewportSize.Y - gazePoint.Value.Y)) / mFOVMargin.W;
					if (ExtendedFOVMode == EExtendedFOVMode::ExtendedScreen)
						speedInterpolation.Y = -speedInterpolation.Y;
				}
			}
			else if (ExtendedFOVMode == EExtendedFOVMode::ExtendedScreen)
			{
				if (gazePoint.Value.X < mFOVMargin.X)
					speedInterpolation.X = -1;
				else if (gazePoint.Value.X >(mViewportSize.X - mFOVMargin.Z))
					speedInterpolation.X = 1;
				if (gazePoint.Value.Y < mFOVMargin.Y)
					speedInterpolation.Y = 1;
				else if (gazePoint.Value.Y >(mViewportSize.Y - mFOVMargin.W))
					speedInterpolation.Y = -1;
			}

			if (ExtendedFOVMode == EExtendedFOVMode::InfiniteScreen)
			{
				InfiniteScreenFOV(relativeGazePoint, speedInterpolation, channelerEyeXController->ExtendedFOVTurnRate);
			}
			else if (ExtendedFOVMode == EExtendedFOVMode::ExtendedScreen)
			{
				ExtendedScreenFOV(relativeGazePoint, speedInterpolation);
			}
		}
		else if (ExtendedFOVMode == EExtendedFOVMode::ExtendedScreen)
		{
			//ExtendedScreenFOV(FVector2D(0, 0), FVector2D(0, 0));
		}
	}

	mMouseWasMoved = false;
}

void AChannelerCharacter::InfiniteScreenFOV(const FVector2D& relativeGazePoint, const FVector2D& speedInterpolation, float turnRate)
{
	FVector2D fovSpeed = FVector2D(relativeGazePoint.X * turnRate * speedInterpolation.X,
		relativeGazePoint.Y * turnRate * speedInterpolation.Y);

	APlayerController* const playerController = GetWorld()->GetFirstPlayerController();

	float deltaYaw = fovSpeed.X * playerController->InputYawScale;
	float deltaPitch = fovSpeed.Y * playerController->InputPitchScale;

	AddControllerYawInput(fovSpeed.X);
	AddControllerPitchInput(fovSpeed.Y);

}

void AChannelerCharacter::ExtendedScreenFOV(const FVector2D& relativeGazePoint, const FVector2D& speedInterpolation)
{
	float inputYawScale = GetWorld()->GetFirstPlayerController()->InputYawScale;
	FVector2D speed = speedInterpolation;

	float yaw = EasingResponsiveness * (ExtendedScreenMaxAngle.X * speed.X - mFOVCameraRotation.Yaw);
	float finalCameraYaw = mFOVCameraRotation.Yaw + yaw;
	if (FMath::Abs(finalCameraYaw) <= ExtendedScreenMaxAngle.X && FMath::Abs(yaw) > ExtendedScreenFilterAngle.X)
	{
		mFOVCameraRotation.Yaw = finalCameraYaw;
		AddControllerYawInput(yaw / inputYawScale);
	}

	FRotator cameraRotation = GetFirstPersonCameraComponent()->RelativeRotation;
	float inputPitchScale = GetWorld()->GetFirstPlayerController()->InputPitchScale;
	float pitch = EasingResponsiveness * (ExtendedScreenMaxAngle.Y * speed.Y - mFOVCameraRotation.Pitch);
	float finalCameraPitch = mFOVCameraRotation.Pitch + pitch;
	if (FMath::Abs(finalCameraPitch) <= ExtendedScreenMaxAngle.Y && FMath::Abs(pitch) > ExtendedScreenFilterAngle.Y)
	{
		mFOVCameraRotation.Pitch = finalCameraPitch;
		AddControllerPitchInput(pitch / inputPitchScale);
	}
}

void AChannelerCharacter::EllipticalExtendedScreenFOV(const FVector2D& relativeGazePoint, float turnRate)
{
	// check if gaze point is inside the inner ellipse
	FVector2D relativePoint = relativeGazePoint;
	FVector2D relativeGazePointSquared = FVector2D(FMath::Square(relativeGazePoint.X), FMath::Square(relativeGazePoint.Y));
	FVector2D fovAngle;
	bool alreadyCalculated = false;
	if ((relativeGazePointSquared.X / mFOVEllipseAxesSquared.X) + (relativeGazePointSquared.Y / mFOVEllipseAxesSquared.Y) < 1)
	{
		fovAngle = FVector2D(0, 0);
		alreadyCalculated = true;
	}
	//check if gaze point is outside the outer ellipse
	if ((relativeGazePointSquared.X / mFOVOuterEllipseAxesSquared.X) + (relativeGazePointSquared.Y / mFOVOuterEllipseAxesSquared.Y) > 1)
	{
		float currentGazeDistance = relativeGazePoint.Size();
		float cosAlpha = relativeGazePoint.X / currentGazeDistance;
		float sinAlpha = relativeGazePoint.Y / currentGazeDistance;
		relativePoint = FVector2D(mFOVOuterEllipseAxes.X * cosAlpha, mFOVOuterEllipseAxes.Y * sinAlpha);
		fovAngle = FVector2D(ExtendedScreenMaxAngle.X, ExtendedScreenMaxAngle.Y);
	}

	// gaze point is inside the extended FOV region
	if (!alreadyCalculated)
	{
		float gazeDistance = relativePoint.Size();
		float cosTheta = relativePoint.X / gazeDistance;
		float sinTheta = relativePoint.Y / gazeDistance;

		FVector2D pointOnOuterEllipse = FVector2D(mFOVOuterEllipseAxes.X * cosTheta, mFOVOuterEllipseAxes.Y * sinTheta);
		FVector2D pointOnInnerEllipse = FVector2D(mFOVEllipseAxes.X * cosTheta, mFOVEllipseAxes.Y * sinTheta);

		float maxGazeDistance = pointOnOuterEllipse.Size();
		float minGazeDistance = pointOnInnerEllipse.Size();

		if (gazeDistance < minGazeDistance || gazeDistance > maxGazeDistance)
			return;

		float extendedFOVRatio = (gazeDistance - minGazeDistance) / maxGazeDistance;
		fovAngle = FVector2D(extendedFOVRatio * cosTheta * ExtendedScreenMaxAngle.X, -(extendedFOVRatio * sinTheta * ExtendedScreenMaxAngle.Y));
	}


	FVector2D deltaAngle = FVector2D(fovAngle.X - mFOVCameraRotation.Yaw, fovAngle.Y - mFOVCameraRotation.Pitch);
	float inputYawScale = GetWorld()->GetFirstPlayerController()->InputYawScale;
	float inputPitchScale = GetWorld()->GetFirstPlayerController()->InputPitchScale;

	deltaAngle *= EasingResponsiveness;

	if (FMath::Abs(deltaAngle.X) > ExtendedScreenFilterAngle.X)
	{
		AddControllerYawInput((deltaAngle.X * turnRate * 2) / inputYawScale);
		mFOVCameraRotation.Yaw += deltaAngle.X;
	}
	if (FMath::Abs(deltaAngle.Y) > ExtendedScreenFilterAngle.Y)
	{
		AddControllerPitchInput((deltaAngle.Y * turnRate * 2) / inputPitchScale);
		mFOVCameraRotation.Pitch += deltaAngle.Y;
	}
}

void AChannelerCharacter::UpdateScreenResolutionRelatedProperties(int32 newWidth, int32 newHeight)
{
	mViewportSize = FIntPoint(newWidth, newHeight);
	mViewportCenter = FIntPoint(mViewportSize.X / 2, mViewportSize.Y / 2);

	mFOVMargin = FVector4(
		mViewportSize.X * ExtendedFOVMargin.Left,
		mViewportSize.Y * ExtendedFOVMargin.Top,
		mViewportSize.X * ExtendedFOVMargin.Right,
		mViewportSize.Y * ExtendedFOVMargin.Bottom
	);

	mFOVEllipseAxes = FVector2D(NoFOVEllipseAxesRatio.X * mViewportSize.X / 2, NoFOVEllipseAxesRatio.Y * mViewportSize.Y / 2);
	mFOVEllipseAxesSquared = FVector2D(FMath::Square(mFOVEllipseAxes.X), FMath::Square(mFOVEllipseAxes.Y));
	mFOVOuterEllipseAxes = FVector2D(mViewportSize.X / FMath::Sqrt(2), mViewportSize.Y / FMath::Sqrt(2));
	mFOVOuterEllipseAxesSquared = FVector2D(FMath::Square(mFOVOuterEllipseAxes.X), FMath::Square(mFOVOuterEllipseAxes.Y));
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "DialogueFreezeCam.h"
#include "../Puzzles/Puzzle.h"
#include "../Characters/ChannelerCharacter.h"
#include "../TheChannelerHUD.h"

ADialogueFreezeCam::ADialogueFreezeCam() :
	CameraName("DefaultName"), mInitializerBeginOverlapDelegate(), mInitializerEndOverlapDelegate(),
	TriggerAngle(15.0f), DisableMovement(false),
	mTransitionTimerHandle(), TransitionOnOverlap(true),
	mCameraState(EDialogueCamState::NONE), mOriginalCamera(nullptr),
	GoDirectlyToPuzzleCam(false), ExitDirectlyToPlayer(false)
{
	PrimaryActorTick.bCanEverTick = false;

	mRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(mRootComponent);

	//mChildActor = SpawnAc<AActor>(TEXT("PuzzleCamActor"));
	mChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("PuzzleCamActorContainer"));
	mChildActorComponent->AttachTo(RootComponent);
	mChildActorComponent->SetChildActorClass(APuzzleCamActor::StaticClass());
	mChildActorComponent->CreateChildActor();
	AActor* actor = mChildActorComponent->ChildActor;
	mChildActor = Cast<APuzzleCamActor>(actor);

	TriggerInitializer = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerInitializer"));
	TriggerInitializer->bGenerateOverlapEvents = true;
	TriggerInitializer->AttachTo(RootComponent);

	TriggerInitializer->SetCollisionProfileName(FName(TEXT("OverlapAll")));

	DialogueCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DialogueCamera"));
	DialogueCamera->AttachTo(RootComponent);

}

void ADialogueFreezeCam::BeginPlay()
{
	Super::BeginPlay();

	mInitializerBeginOverlapDelegate.BindUFunction(this, "InitializerOnBeginOverlap");
	TriggerInitializer->OnComponentBeginOverlap.AddUnique(mInitializerBeginOverlapDelegate);

	mInitializerEndOverlapDelegate.BindUFunction(this, "InitializerOnEndOverlap");
	TriggerInitializer->OnComponentEndOverlap.AddUnique(mInitializerEndOverlapDelegate);


	if (TargetPuzzle != nullptr)
	{
		TargetPuzzle->SetFreezeCam(*this);
	}

	AActor* actor = mChildActorComponent->ChildActor;
	mChildActor = Cast<APuzzleCamActor>(actor);
}

void ADialogueFreezeCam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADialogueFreezeCam::InitializerOnBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AChannelerCharacter>(OtherActor) == nullptr)
		return;

	UE_LOG(LogTemp, Warning, TEXT("ADialogueFreezeCam::InitializerOnBeginOverlap() %s %s"), *CameraName, *(OtherActor->GetName()));
	mCameraState = EDialogueCamState::IDLE;

	FVector forwardVector = GetActorForwardVector();
	FVector playerForwardVector = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorForwardVector();
	forwardVector.Z = 0;
	playerForwardVector.Z = 0;

	float dotProduct = FVector::DotProduct(forwardVector, playerForwardVector);
	float angle = FMath::RadiansToDegrees(FMath::Acos(dotProduct));

	//UE_LOG(LogTemp, Warning, TEXT("Angle = %f"), angle);

	if (angle <= TriggerAngle || !UseTriggerAngle)
	{
		if (TransitionOnOverlap)
		{
			if (GoDirectlyToPuzzleCam)
				TransitionToPuzzleCam();
			else
			{
				TransitionToDialogueCam();
			}				
		}
	}
}

void ADialogueFreezeCam::InitializerOnEndOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AChannelerCharacter>(OtherActor) == nullptr)
		return;

	UE_LOG(LogTemp, Warning, TEXT("ADialogueFreezeCam::InitializerOnEndOverlap() %s %s"), *CameraName, *(OtherActor->GetName()));
	mCameraState = EDialogueCamState::NONE;
	if (TargetPuzzle != nullptr && !DisableMovement)
	{
		TargetPuzzle->PausePuzzle();
	}
}

void ADialogueFreezeCam::TransitionToDialogueCam()
{
	UE_LOG(LogTemp, Warning, TEXT("ADialogueFreezeCam::TransitionToDialogueCam() %s cam state = %d "), *CameraName, (int32)mCameraState);
	if (mCameraState == EDialogueCamState::IDLE)
	{
		if (mOriginalCamera == nullptr)
		{
			AActor* actor = GetWorld()->GetFirstPlayerController()->GetPawn();
			mOriginalCamera = Cast<AChannelerCharacter>(actor);
		}
		mOriginalCamera->DisableMovement();
		mOriginalCamera->DisableLook();

		CameraTransition(*this, DialogueCamEntryTransition);
		mTimerDelegate.BindUFunction(this, FName("ActivateDialogueCamState"), EDialogueCamState::DIALOGUE_CAM_ACTIVE);
		GetWorldTimerManager().SetTimer(mTransitionTimerHandle, mTimerDelegate, DialogueCamEntryTransition.CameraBlendTime, false);
	}
	else if (mCameraState == EDialogueCamState::PUZZLE_CAM_ACTIVE)
	{
		CameraTransition(*this, PuzzleCamExitTransition);
		mTimerDelegate.BindUFunction(this, FName("ActivateDialogueCamState"), EDialogueCamState::DIALOGUE_CAM_ACTIVE);
		GetWorldTimerManager().SetTimer(mTransitionTimerHandle, mTimerDelegate, PuzzleCamExitTransition.CameraBlendTime, false);
	}

	AChannelerCharacter* ChannelerCharacter =
		Cast<AChannelerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	ChannelerCharacter->DisableEagleEye();
}

void ADialogueFreezeCam::TransitionToPuzzleCam()
{
	if (mCameraState == EDialogueCamState::DIALOGUE_CAM_ACTIVE || GoDirectlyToPuzzleCam)
	{
		CameraTransition(*mChildActor, PuzzleCamExitTransition);
		mTimerDelegate.BindUFunction(this, FName("ActivateDialogueCamState"), EDialogueCamState::PUZZLE_CAM_ACTIVE);
		GetWorldTimerManager().SetTimer(mTransitionTimerHandle, mTimerDelegate, PuzzleCamEntryTransition.CameraBlendTime, false);
	}
}

void ADialogueFreezeCam::StartPuzzle()
{
	if (mCameraState == EDialogueCamState::PUZZLE_CAM_ACTIVE && TargetPuzzle != nullptr)
	{
		AChannelerCharacter* ChannelerCharacter = 
			Cast<AChannelerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		ChannelerCharacter->SetIsInPuzzle(true);
		ChannelerCharacter->DisableEagleEye();

		ATheChannelerHUD* ChannelerHud = Cast<ATheChannelerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		if (ChannelerHud != nullptr)
		{
			ChannelerHud->TurnOnGaze();
		}

		if (IEyeXPlugin::IsAvailable())
		{
			IEyeXPlugin::Get().SetEmulationPointType(EEyeXEmulationPoint::MousePosition);
		}

		TargetPuzzle->StartPuzzle();
	}
}

void ADialogueFreezeCam::TransferControlToPlayer()
{
	if (IEyeXPlugin::IsAvailable())
	{
		IEyeXPlugin::Get().SetEmulationPointType(EEyeXEmulationPoint::ScreenCenter);
	}

	if (ExitDirectlyToPlayer)
	{
		EndInteraction();
	}
	else
		TransitionToDialogueCam();
}

void ADialogueFreezeCam::EndInteraction()
{
	FCameraTransition* camTransition = nullptr;
	if (mCameraState == EDialogueCamState::DIALOGUE_CAM_ACTIVE)
	{
		camTransition = &DialogueCamExitTransition;
	}
	else if (mCameraState == EDialogueCamState::PUZZLE_CAM_ACTIVE)
	{
		camTransition = &PuzzleCamExitTransition;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Dialogue Freeze Cam - Invalid state to end interaction."));
		return;
	}

	CameraTransition(*mOriginalCamera, *camTransition);
	mTimerDelegate.BindUFunction(this, FName("ActivateDialogueCamState"), EDialogueCamState::IDLE);
	GetWorldTimerManager().SetTimer(mTransitionTimerHandle, mTimerDelegate, camTransition->CameraBlendTime, false);
	mOriginalCamera->EnableMovement();
	mOriginalCamera->EnableLook();

	AChannelerCharacter* ChannelerCharacter =
		Cast<AChannelerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	ChannelerCharacter->EnableEagleEye();
	if (!ChannelerCharacter->IsInPuzzle())
	{
		ATheChannelerHUD* ChannelerHud = Cast<ATheChannelerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		if (ChannelerHud != nullptr)
		{
			ChannelerHud->TurnOffGaze();
		}
	}
}

void ADialogueFreezeCam::DisableFreezeCam()
{
	TriggerInitializer->OnComponentBeginOverlap.Remove(mInitializerBeginOverlapDelegate);
	TriggerInitializer->OnComponentEndOverlap.Remove(mInitializerEndOverlapDelegate);
	UE_LOG(LogTemp, Warning, TEXT("Disabled %s"), *CameraName);
}

void ADialogueFreezeCam::EnableFreezeCam()
{
	TriggerInitializer->OnComponentBeginOverlap.AddUnique(mInitializerBeginOverlapDelegate);
	TriggerInitializer->OnComponentEndOverlap.AddUnique(mInitializerEndOverlapDelegate);
}

void ADialogueFreezeCam::CameraTransition(AActor& targetActor, const FCameraTransition& cameraTransition)
{
	GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(&targetActor, cameraTransition.CameraBlendTime, cameraTransition.CameraBlendFunction, cameraTransition.CameraBlendExponent);
}

void ADialogueFreezeCam::ActivateDialogueCamState(EDialogueCamState newState)
{
	EDialogueCamState oldState = mCameraState;
	mCameraState = newState;
	mTimerDelegate.Unbind();
	GetWorld()->GetTimerManager().ClearTimer(mTransitionTimerHandle);

	if (newState == EDialogueCamState::IDLE)
	{
		mOriginalCamera->ActiveCameraComponent = mOriginalCamera->GetFirstPersonCameraComponent();
		AChannelerCharacter* ChannelerCharacter =
			Cast<AChannelerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		ChannelerCharacter->EnableEagleEye();
	}
	else if (newState == EDialogueCamState::DIALOGUE_CAM_ACTIVE)
	{
		mOriginalCamera->ActiveCameraComponent = DialogueCamera;
		AChannelerCharacter* ChannelerCharacter =
			Cast<AChannelerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		ChannelerCharacter->DisableEagleEye();
	}
	else if (newState == EDialogueCamState::PUZZLE_CAM_ACTIVE)
	{
		mOriginalCamera->ActiveCameraComponent = mChildActor->PuzzleCamera;
		AChannelerCharacter* ChannelerCharacter =
			Cast<AChannelerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		ChannelerCharacter->DisableEagleEye();
	}

	if (CameraTransitionComplete.IsBound())
		CameraTransitionComplete.Broadcast(oldState, newState);
}

void ADialogueFreezeCam::UpdateCameraState(EDialogueCamState newState)
{
	mCameraState = newState;
}
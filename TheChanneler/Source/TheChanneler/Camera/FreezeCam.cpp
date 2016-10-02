// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "FreezeCam.h"
#include "../Puzzles/Puzzle.h"
#include "../Characters/ChannelerCharacter.h"
#include "../TheChannelerHUD.h"

FCameraTransition::FCameraTransition() :
	CameraBlendTime(1.0f), CameraBlendFunction(EViewTargetBlendFunction::VTBlend_Linear), CameraBlendExponent(0)
{}

AFreezeCam::AFreezeCam() :
	mInitializerBeginOverlapDelegate(), mInitializerEndOverlapDelegate(),
	TriggerAngle(15.0f), DisableMovement(false), PosessOnBeginPlay(false),
	mTransitionTimerHandle()
{
	PrimaryActorTick.bCanEverTick = false;

	mRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(mRootComponent);

	TriggerInitializer = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerInitializer"));
	TriggerInitializer->bGenerateOverlapEvents = true;
	TriggerInitializer->AttachTo(RootComponent);

	TriggerInitializer->SetCollisionProfileName(FName(TEXT("OverlapAll")));

	TargetCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Puzzle Target Camera"));
	TargetCamera->AttachTo(RootComponent);
}

void AFreezeCam::BeginPlay()
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

	if (PosessOnBeginPlay)
	{
		DisableMovement = true;
		ActivateFreezeCam();
	}
}

void AFreezeCam::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AFreezeCam::InitializerOnBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AChannelerCharacter>(OtherActor) == nullptr)
		return;
	UE_LOG(LogTemp, Warning, TEXT("AFreezeCam::InitializerOnBeginOverlap()"));

	FVector forwardVector = GetActorForwardVector();
	FVector playerForwardVector = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorForwardVector();
	forwardVector.Z = 0;
	playerForwardVector.Z = 0;

	float dotProduct = FVector::DotProduct(forwardVector, playerForwardVector);
	float angle = FMath::RadiansToDegrees(FMath::Acos(dotProduct));

	UE_LOG(LogTemp, Warning, TEXT("Angle = %f"), angle);

	if (angle <= TriggerAngle || !UseTriggerAngle)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trigger Puzzle Start"));

		//GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(this, CameraBlendTime, CameraBlendFunction, CameraBlendExponent);
		if (TargetPuzzle != nullptr)
		{
			ActivateFreezeCam();
		}
	}
}

void AFreezeCam::InitializerOnEndOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AChannelerCharacter>(OtherActor) == nullptr)
		return;
	UE_LOG(LogTemp, Warning, TEXT("AFreezeCam::InitializerOnEndOverlap()"));
	if (TargetPuzzle != nullptr && !DisableMovement)
	{
		GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(GetWorld()->GetFirstPlayerController()->GetPawn(),
			ExitCameraTransition.CameraBlendTime, ExitCameraTransition.CameraBlendFunction, ExitCameraTransition.CameraBlendExponent);
		TargetPuzzle->PausePuzzle();
	}
}

void AFreezeCam::TransferControlToPlayer()
{
	APawn* pawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(pawn, ExitCameraTransition.CameraBlendTime, ExitCameraTransition.CameraBlendFunction, ExitCameraTransition.CameraBlendExponent);

	AChannelerCharacter* fpCharacter = Cast<AChannelerCharacter>(pawn);
	if (fpCharacter != nullptr)
	{
		fpCharacter->ActiveCameraComponent = fpCharacter->GetFirstPersonCameraComponent();
		fpCharacter->EnableMovement();
		fpCharacter->EnableLook();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AFreezeCam::InitializerOnBeginOverlap() -> Player controller pawn is not a AChannelerCharacter"));
	}

	if (IEyeXPlugin::IsAvailable())
	{
		IEyeXPlugin::Get().SetEmulationPointType(EEyeXEmulationPoint::ScreenCenter);
	}
}

void AFreezeCam::ActivateFreezeCam()
{
	APawn* pawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	AChannelerCharacter* fpCharacter = Cast<AChannelerCharacter>(pawn);
	if (fpCharacter != nullptr)
	{
		// disabling only while the camera transition takes place
		fpCharacter->DisableMovement();
		fpCharacter->DisableLook();

		GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(this, EntryCameraTransition.CameraBlendTime, EntryCameraTransition.CameraBlendFunction, EntryCameraTransition.CameraBlendExponent);
		fpCharacter->ActiveCameraComponent = TargetCamera;
		//TargetPuzzle->StartPuzzle();
		GetWorldTimerManager().SetTimer(mTransitionTimerHandle, this, &AFreezeCam::StartPuzzle, EntryCameraTransition.CameraBlendTime, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AFreezeCam::InitializerOnBeginOverlap() -> Player controller pawn is not a AChannelerCharacter"));
	}
}

void AFreezeCam::StartPuzzle()
{
	UE_LOG(LogTemp, Warning, TEXT("AFreezeCam::StartPuzzle()"));
	if (!DisableMovement)
	{
		APawn* pawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		AChannelerCharacter* fpCharacter = Cast<AChannelerCharacter>(pawn);
		fpCharacter->EnableMovement();
		fpCharacter->EnableLook();
		fpCharacter->SetIsInPuzzle(true);
	}
	
	if (IEyeXPlugin::IsAvailable())
	{
		IEyeXPlugin::Get().SetEmulationPointType(EEyeXEmulationPoint::MousePosition);
	}

	ATheChannelerHUD* ChannelerHud = Cast<ATheChannelerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	if (ChannelerHud != nullptr)
	{
		ChannelerHud->TurnOnGaze();
	}

	TargetPuzzle->StartPuzzle();
}

void AFreezeCam::DisableFreezeCam()
{
	TriggerInitializer->OnComponentBeginOverlap.Remove(mInitializerBeginOverlapDelegate);
	TriggerInitializer->OnComponentEndOverlap.Remove(mInitializerEndOverlapDelegate);
}

void AFreezeCam::EnableFreezeCam()
{
	TriggerInitializer->OnComponentBeginOverlap.AddUnique(mInitializerBeginOverlapDelegate);
	TriggerInitializer->OnComponentEndOverlap.AddUnique(mInitializerEndOverlapDelegate);
}

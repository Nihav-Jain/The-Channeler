// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../Puzzles/Puzzle.h"
#include "../Characters/ChannelerCharacter.h"
#include "FreeLookCam.h"


// Sets default values
AFreeLookCam::AFreeLookCam() :
	mInitializerBeginOverlapDelegate(), mInitializerEndOverlapDelegate(),
	TriggerAngle(15.0f)
{
	PrimaryActorTick.bCanEverTick = false;

	mRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(mRootComponent);

	TriggerInitializer = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerInitializer"));
	TriggerInitializer->bGenerateOverlapEvents = true;
	TriggerInitializer->AttachTo(RootComponent);

	TriggerInitializer->SetCollisionProfileName(FName(TEXT("OverlapAll")));
}

// Called when the game starts or when spawned
void AFreeLookCam::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("AFreeLookCam::BeginPlay()"));
	Super::BeginPlay();

	mInitializerBeginOverlapDelegate.BindUFunction(this, "InitializerOnBeginOverlap");
	TriggerInitializer->OnComponentBeginOverlap.AddUnique(mInitializerBeginOverlapDelegate);

	mInitializerEndOverlapDelegate.BindUFunction(this, "InitializerOnEndOverlap");
	TriggerInitializer->OnComponentEndOverlap.AddUnique(mInitializerEndOverlapDelegate);


	if (TargetPuzzle != nullptr)
	{
		TargetPuzzle->SetFreezeCam(*this);
	}
}

// Called every frame
void AFreeLookCam::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AFreeLookCam::InitializerOnBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AChannelerCharacter>(OtherActor) == nullptr)
		return;

	UE_LOG(LogTemp, Warning, TEXT("AFreeLookCam::InitializerOnBeginOverlap()"));

	FVector forwardVector = GetActorForwardVector();
	FVector playerForwardVector = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorForwardVector();
	forwardVector.Z = 0;
	playerForwardVector.Z = 0;

	float dotProduct = FVector::DotProduct(forwardVector, playerForwardVector);
	float angle = FMath::RadiansToDegrees(FMath::Acos(dotProduct));

	UE_LOG(LogTemp, Warning, TEXT("Angle = %f"), angle);

	if (angle <= TriggerAngle)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trigger Puzzle Start"));

		if (TargetPuzzle != nullptr)
		{
			ActivateFreeLookCam();
		}
	}
}

void AFreeLookCam::InitializerOnEndOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AChannelerCharacter>(OtherActor) == nullptr)
		return;
	UE_LOG(LogTemp, Warning, TEXT("AFreeLookCam::InitializerOnEndOverlap()"));
	if (TargetPuzzle != nullptr)
	{
		TargetPuzzle->PausePuzzle();
	}
}

void AFreeLookCam::TransferControlToPlayer()
{
	APawn* pawn = GetWorld()->GetFirstPlayerController()->GetPawn();

	AChannelerCharacter* fpCharacter = Cast<AChannelerCharacter>(pawn);
	if (fpCharacter != nullptr)
	{
		fpCharacter->ActiveCameraComponent = fpCharacter->GetFirstPersonCameraComponent();
		fpCharacter->EnableMovement();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AFreezeCam::InitializerOnBeginOverlap() -> Player controller pawn is not a AChannelerCharacter"));
	}
}

void AFreeLookCam::ActivateFreeLookCam()
{
	APawn* pawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	AChannelerCharacter* fpCharacter = Cast<AChannelerCharacter>(pawn);
	if (fpCharacter != nullptr)
	{
		fpCharacter->DisableMovement();
		TargetPuzzle->StartPuzzle();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AFreeLookCam::InitializerOnBeginOverlap() -> Player controller pawn is not a AChannelerCharacter"));
	}
}

void AFreeLookCam::DisableFreezeCam()
{
	TriggerInitializer->OnComponentBeginOverlap.Remove(mInitializerBeginOverlapDelegate);
	TriggerInitializer->OnComponentEndOverlap.Remove(mInitializerEndOverlapDelegate);
}

void AFreeLookCam::EnableFreezeCam()
{
	TriggerInitializer->OnComponentBeginOverlap.AddUnique(mInitializerBeginOverlapDelegate);
	TriggerInitializer->OnComponentEndOverlap.AddUnique(mInitializerEndOverlapDelegate);
}

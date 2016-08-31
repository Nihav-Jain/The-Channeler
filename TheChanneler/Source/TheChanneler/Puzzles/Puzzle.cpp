// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "Puzzle.h"
#include "../Camera/FreezeCam.h"
#include "../Kernel/GameInstanceBase.h"
#include "../Characters/ChannelerCharacter.h"
#include "../TheChannelerHUD.h"

APuzzle::APuzzle()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APuzzle::BeginPlay()
{
	Super::BeginPlay();
}

void APuzzle::Tick( float DeltaTime )
{
	Super::Tick(DeltaTime);
}

void APuzzle::SetFreezeCam(APuzzleCam& freezeCam)
{
	mFreezeCam = &freezeCam;
}

void APuzzle::EndPuzzle()
{
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();

	APawn* pawn = playerController->GetPawn();
	AChannelerCharacter* fpCharacter = Cast<AChannelerCharacter>(pawn);
	if (fpCharacter != nullptr)
	{
		fpCharacter->SetIsInPuzzle(false);
	}
	
	ATheChannelerHUD* ChannelerHud = Cast<ATheChannelerHUD>(playerController->GetHUD());
	if (ChannelerHud != nullptr)
	{
		ChannelerHud->TurnOffGaze();
	}

	if(mFreezeCam != nullptr)
		mFreezeCam->TransferControlToPlayer();
}

void APuzzle::StartPuzzle_Implementation()
{
	unimplemented();
}

void APuzzle::PausePuzzle_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("APuzzle::PausePuzzle() native implementation"));
}

void APuzzle::DisableMyFreezeCam()
{
	if(mFreezeCam != nullptr)
		mFreezeCam->DisableFreezeCam();
}

void APuzzle::EnableMyFreezeCam()
{
	if (mFreezeCam != nullptr)
		mFreezeCam->EnableFreezeCam();
}

const FString& APuzzle::GetPuzzleName() const
{
	return PuzzleName;
}

void APuzzle::SetPuzzleName(const FString& name)
{
	PuzzleName = name;
}
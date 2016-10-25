// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "ChannelerCharacter.h"
#include "GhostCameraActor.h"

AGhostCameraActor::AGhostCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;

	mRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(mRootComponent);

	GhostCam = CreateDefaultSubobject<UCameraComponent>(TEXT("GhostCam"));
	GhostCam->AttachParent = GetRootComponent();
	GhostCam->RelativeLocation = FVector(0, 0, 0); // Position the camera
	GhostCam->bUsePawnControlRotation = false;
}

void AGhostCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
	mController = GetWorld()->GetFirstPlayerController();
	mChanneler = Cast<AChannelerCharacter>(mController->GetPawn());

	mController->SetViewTarget(this);
}

void AGhostCameraActor::Tick( float DeltaTime )
{
	Super::Tick(DeltaTime);

	FRotator channelerRotation = mChanneler->GetActorRotation();
	FRotator channelerViewRotation = mChanneler->GetCharacterViewRotation();

	FRotator ghostCamActorRotation = FRotator(channelerViewRotation.Pitch, channelerRotation.Yaw, channelerRotation.Roll);
	FVector ghostCamActorLocation = mChanneler->GetActorLocation();
	ghostCamActorLocation.Z += 64.0f;
	UE_LOG(LogTemp, Warning, TEXT("1. %f %f %f"), GhostCam->RelativeRotation.Yaw, GhostCam->RelativeRotation.Pitch, GhostCam->RelativeRotation.Roll);
	SetActorLocationAndRotation(ghostCamActorLocation, ghostCamActorRotation);
	UE_LOG(LogTemp, Warning, TEXT("2. %f %f %f"), GhostCam->RelativeRotation.Yaw, GhostCam->RelativeRotation.Pitch, GhostCam->RelativeRotation.Roll);
}


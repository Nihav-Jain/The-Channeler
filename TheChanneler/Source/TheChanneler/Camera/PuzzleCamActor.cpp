// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "PuzzleCamActor.h"


// Sets default values
APuzzleCamActor::APuzzleCamActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	mRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(mRootComponent);

	PuzzleCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PuzzleCamera"));
	PuzzleCamera->AttachTo(RootComponent);

}

// Called when the game starts or when spawned
void APuzzleCamActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APuzzleCamActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}


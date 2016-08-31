// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "MoveToTargetActor.h"


// Sets default values
AMoveToTargetActor::AMoveToTargetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMoveToTargetActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMoveToTargetActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}


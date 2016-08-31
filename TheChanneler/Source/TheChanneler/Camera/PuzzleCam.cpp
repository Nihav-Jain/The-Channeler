// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "PuzzleCam.h"


// Sets default values
APuzzleCam::APuzzleCam() :
	TargetPuzzle(nullptr), UseTriggerAngle(true)
{
	PrimaryActorTick.bCanEverTick = false;
}



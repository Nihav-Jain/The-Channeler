// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PuzzleCam.generated.h"

UCLASS(abstract)
class THECHANNELER_API APuzzleCam : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APuzzleCam();

	virtual void DisableFreezeCam() {};

	virtual void EnableFreezeCam() {};

	/**
	*	Called by Puzzle::EndPuzzle() function, switched camera to Player Camera and renables Character Movement
	*/
	virtual void TransferControlToPlayer() {};

	/** Puzzle to be enabled by this freeze cam */
	UPROPERTY(EditInstanceOnly, Category = "FreezeCam")
	class APuzzle* TargetPuzzle;

	UPROPERTY(EditAnywhere, Category = "FreezeCam")
	bool UseTriggerAngle;
};

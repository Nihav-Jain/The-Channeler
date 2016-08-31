// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PuzzleCamActor.generated.h"

UCLASS()
class THECHANNELER_API APuzzleCamActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APuzzleCamActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	/** View will shift to this Camera when the Puzzle starts */
	UPROPERTY(EditInstanceOnly)
	UCameraComponent* PuzzleCamera;

private:
	USceneComponent* mRootComponent;

};

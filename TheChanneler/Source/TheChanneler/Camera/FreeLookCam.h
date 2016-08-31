// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PuzzleCam.h"
#include "FreeLookCam.generated.h"

UCLASS()
class THECHANNELER_API AFreeLookCam : public APuzzleCam
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFreeLookCam();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/** Triggers the start of the Puzzle */
	UPROPERTY(EditInstanceOnly)
	UBoxComponent* TriggerInitializer;

	/** Angle range, as calcuated relative to the forward vector of this Actor, in which the puzzle must be activated */
	UPROPERTY(EditAnywhere, Category = "FreezeCam", meta = (ClampMin = "0", ClampMax = "180"))
	float TriggerAngle;

	virtual void DisableFreezeCam() override;

	virtual void EnableFreezeCam() override;

	/**
	*	Called by Puzzle::EndPuzzle() function, switched camera to Player Camera and renables Character Movement
	*/
	virtual void TransferControlToPlayer() override;

private:
	/**
	*	BoxTrigger OnBeginOverlap callback (needs to be a UFUNCTION to be called by FScriptDelegate)
	*/
	UFUNCTION()
	void InitializerOnBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	*	BoxTrigger OnEndOverlap callback (needs to be a UFUNCTION to be called by FScriptDelegate)
	*/
	UFUNCTION()
	void InitializerOnEndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ActivateFreeLookCam();

	/** Root component for this Actor - serves no purpose specific to FreezeCam */
	USceneComponent* mRootComponent;

	/** Delegate object to hold the OnBeginOverlap callback info */
	FScriptDelegate mInitializerBeginOverlapDelegate;

	/** Delegate object to hold the OnEndOverlap callback info */
	FScriptDelegate mInitializerEndOverlapDelegate;

};

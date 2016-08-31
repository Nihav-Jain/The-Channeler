// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PuzzleCam.h"
#include "FreezeCam.generated.h"

USTRUCT()
struct FCameraTransition
{
	GENERATED_BODY()
public:
	FCameraTransition();

	/** Time taken to blend between the 2 cameras */
	UPROPERTY(EditAnywhere, meta = (DefaultValue = "1", ClampMin = "0"))
	float CameraBlendTime;

	/** Camera switching blend mode */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EViewTargetBlendFunction> CameraBlendFunction;

	/** Exponent to enhave the blend fucntion */
	UPROPERTY(EditAnywhere)
	float CameraBlendExponent;
};

/**
 *	Consists of a trigger volume and a still camera, to be used for triggering puzzles which require player movement to be disabled
 */
UCLASS()
class THECHANNELER_API AFreezeCam : public APuzzleCam
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFreezeCam();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/** Triggers the start of the Puzzle */
	UPROPERTY(EditInstanceOnly)
	UBoxComponent* TriggerInitializer;

	/** View will shift to this Camera when the Puzzle starts */
	UPROPERTY(EditInstanceOnly)
	UCameraComponent* TargetCamera;

	/** Angle range, as calcuated relative to the forward vector of this Actor, in which the puzzle must be activated */
	UPROPERTY(EditAnywhere, Category = "FreezeCam", meta=(ClampMin="0", ClampMax="180"))
	float TriggerAngle;

	/** Should the player movement be disabled when the Puzzle starts? */
	UPROPERTY(EditInstanceOnly, Category = "FreezeCam")
	bool DisableMovement;

	/** Start the puzzle when the game starts (TEMPORARY - only for prototyping period, will possibly be removed in the main game) */
	UPROPERTY(EditInstanceOnly, Category = "FreezeCam")
	bool PosessOnBeginPlay;

	UPROPERTY(EditInstanceOnly, Category = "FreezeCam")
	FCameraTransition EntryCameraTransition;

	UPROPERTY(EditInstanceOnly, Category = "FreezeCam")
	FCameraTransition ExitCameraTransition;

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

	void ActivateFreezeCam();
	void StartPuzzle();

	/** Root component for this Actor - serves no purpose specific to FreezeCam */
	USceneComponent* mRootComponent;

	/** Delegate object to hold the OnBeginOverlap callback info */
	FScriptDelegate mInitializerBeginOverlapDelegate;

	/** Delegate object to hold the OnEndOverlap callback info */
	FScriptDelegate mInitializerEndOverlapDelegate;
	FTimerHandle mTransitionTimerHandle;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/PuzzleCam.h"
#include "FreezeCam.h"
#include "PuzzleCamActor.h"
#include "DialogueFreezeCam.generated.h"

UENUM(BlueprintType)
enum class EDialogueCamState : uint8
{
	NONE,
	IDLE,
	DIALOGUE_CAM_ACTIVE,
	PUZZLE_CAM_ACTIVE
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCameraTransitionComplete, EDialogueCamState, OldState, EDialogueCamState, NewState);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class THECHANNELER_API ADialogueFreezeCam : public APuzzleCam
{
	GENERATED_BODY()
	
public:
	ADialogueFreezeCam();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditInstanceOnly)
	FString CameraName;

	/** Triggers the start of the Puzzle */
	UPROPERTY(EditInstanceOnly)
	UBoxComponent* TriggerInitializer;
	
	UPROPERTY(EditInstanceOnly)
	UChildActorComponent* mChildActorComponent;

	UPROPERTY(EditInstanceOnly)
	UCameraComponent* DialogueCamera;

	/** Angle range, as calcuated relative to the forward vector of this Actor, in which the puzzle must be activated */
	UPROPERTY(EditAnywhere, Category = "DialogueFreezeCam", meta = (ClampMin = "0", ClampMax = "180"))
	float TriggerAngle;

	/** Should the player movement be disabled when the Puzzle starts? */
	UPROPERTY(EditInstanceOnly, Category = "DialogueFreezeCam")
	bool DisableMovement;

	UPROPERTY(EditInstanceOnly, Category = "DialogueFreezeCam")
	bool TransitionOnOverlap;

	UPROPERTY(EditInstanceOnly, Category = "DialogueFreezeCam")
	bool GoDirectlyToPuzzleCam;

  UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "DialogueFreezeCam")
	bool ExitDirectlyToPlayer;

	UPROPERTY(EditInstanceOnly, Category = "DialogueFreezeCam")
	FCameraTransition DialogueCamEntryTransition;

	UPROPERTY(EditInstanceOnly, Category = "DialogueFreezeCam")
	FCameraTransition DialogueCamExitTransition;

	UPROPERTY(EditInstanceOnly, Category = "DialogueFreezeCam")
	FCameraTransition PuzzleCamEntryTransition;

	UPROPERTY(EditInstanceOnly, Category = "DialogueFreezeCam")
	FCameraTransition PuzzleCamExitTransition;

	UPROPERTY(BlueprintAssignable, Category = "Storytelling")
	FCameraTransitionComplete CameraTransitionComplete;
		
	UFUNCTION(BlueprintCallable, Category = "DialogueFreezeCam")
	void TransitionToDialogueCam();

	UFUNCTION(BlueprintCallable, Category = "DialogueFreezeCam")
	void TransitionToPuzzleCam();

	UFUNCTION(BlueprintCallable, Category = "DialogueFreezeCam")
	void StartPuzzle();

	UFUNCTION(BlueprintCallable, Category = "DialogueFreezeCam")
	void EndInteraction();

	UFUNCTION(BlueprintCallable, Category = "DialogueFreezeCam")
	void UpdateCameraState(EDialogueCamState newState);

	virtual void TransferControlToPlayer() override;

	virtual void DisableFreezeCam() override;

	virtual void EnableFreezeCam() override;

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

	//void InternalCameraTransition(UCameraComponent& activateCamera, UCameraComponent& deactivateCamera, const FCameraTransition& cameraTransition);
	void CameraTransition(AActor& targetActor, const FCameraTransition& cameraTransition);

	UFUNCTION()
	void ActivateDialogueCamState(EDialogueCamState newState);

	//void ActivateFreezeCam();
	//void StartPuzzle();

	/** Root component for this Actor - serves no purpose specific to FreezeCam */
	USceneComponent* mRootComponent;

	APuzzleCamActor* mChildActor;

	/** Delegate object to hold the OnBeginOverlap callback info */
	FScriptDelegate mInitializerBeginOverlapDelegate;

	/** Delegate object to hold the OnEndOverlap callback info */
	FScriptDelegate mInitializerEndOverlapDelegate;
	FTimerHandle mTransitionTimerHandle;
	FTimerDelegate mTimerDelegate;

	EDialogueCamState mCameraState;

	AChannelerCharacter* mOriginalCamera;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Puzzle.generated.h"

/**
 *	Abstract class to represent a Puzzle
 *	Must implement the StartPuzzle() and PausePuzzle() functions
 */
UCLASS(abstract)
class THECHANNELER_API APuzzle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APuzzle();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/**
	 *	Called by FreezeCam when the player enters the puzzle trigger
	 *	IMPORTANT!!! Pure virtual function, needs to be implemented by the child
	 */
	UFUNCTION(BlueprintNativeEvent)
	void StartPuzzle();

	/**
	 *	[Blueprint Callable] Should be called by Puzzle if it wants to disable the trigger of its freeze cam
	 */
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void DisableMyFreezeCam();

	/**
	 *	[Blueprint Callable] Should be called by Puzzle if it wants to re-enable the trigger of its freeze cam. Trigger is enabled by default
	 */
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void EnableMyFreezeCam();

	/**
	 *	Called by FreezeCam when the player exits the puzzle trigger
	 *	If the Puzzle has any timers, event dispatchers - pause them in this function
	 */
	UFUNCTION(BlueprintNativeEvent)
	void PausePuzzle();

	/**
	 *	Mutator for the member variable representing the FreezeCam which enables this Puzzle
	 *	@param reference to the FreezeCam object
	 */
	void SetFreezeCam(class APuzzleCam& freezeCam);

	/**
	 * Returns the name of the current puzzle.
	 * @return the name of the current puzzle
	 */
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	const FString& GetPuzzleName() const;

	/**
	 * Sets the name of the current puzzle.
	 * @param the name of the current puzzle
	 */
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void SetPuzzleName(const FString& name);

protected:

	/**
	 *	Must be called by the Puzzle when it wants to transfer control back to the Player
	 */
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void EndPuzzle();

	/**
	 *	Native implementation of StartPuzzle()
	 *	Body declares the PURE_VIRTUAL macro
	 */
	void StartPuzzle_Implementation();

	/**
	 *	Native implementation of PausePuzzle()
	 *	Body declares the PURE_VIRTUAL macro
	 */
	void PausePuzzle_Implementation();

private:

	/** FreezeCam which enables this Puzzle */
	class APuzzleCam* mFreezeCam;

	FString PuzzleName;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "GameFramework/GameMode.h"
#include <functional>
#include "EyeXEx/EyeXPluginEx.h"
#include "TheChannelerGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAppRegainedFocus);

/**
* GameMode - used to manage the widget forms and save/read file contents.
*/
UCLASS()
class THECHANNELER_API ATheChannelerGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATheChannelerGameMode();

	/**
	* BeginPlay - Called when the game starts.
	*/
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, Category="EyeXEx")
	FAppRegainedFocus AppRegainedFocus;

	/**
	* ChangeMenuWidget - Remove the current menu widget and create a new one from the specified class, if provided.
	* @param NewWidgetClass the new widget form to display
	*/
	UFUNCTION(BlueprintCallable, Category = "UMG Game")
		void ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "UMG Game")
		UUserWidget* GetCurrentMenuWidget();

	/**
	* ReadFile - Reads the file with the given name in the string provided.
	* @param OutStringData file is read and stored in this variable
	* @param FileNameWithExtension is taken as string from blueprints
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|Tobii")
		void LaunchCalibration(const FString& OutStringData);

	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|Tobii")
		FString GetPlayerName();

	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|Tobii")
		TArray<FString> GetProfiles();

	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|Tobii")
		FString GetCurrentProfile();


	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|Tobii")
		void SetProfile(const FString& Profile);


	/**
	*	Called by FreezeCam when the player enters the puzzle trigger
	*	IMPORTANT!!! Pure virtual function, needs to be implemented by the child
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Channeler|GameMode|Event")
		void UpdateProfile();



	/**
	* ReadFile - Reads the file with the given name in the string provided.
	* @param OutStringData file is read and stored in this variable
	* @param FileNameWithExtension is taken as string from blueprints
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|File")
		void ReadFile(FString& OutStringData, FString FileNameWithExtension);

	/**
	* SaveFile - Saves/overwrites a file with given string and name
	* @param StringData line is read and stored in this variable
	* @param FileNameWithExtension
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|File")
		void SaveFile(FString StringData, FString FileNameWithExtension);


	/**
	* StringAtLineNumber - returns a string at a specific line number in Data
	* @param OutData line is read and stored in this variable
	* @param LineNumber
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|File")
		void StringAtLineNumber(FString& OutData, int32 LineNumber);

	/**
	* StringAtColumnNumber - returns a string at a specific column number in Data
	* @param InputData input string
	* @param OutData data is returned as a reference
	* @param ColumnNumber
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|File")
		void StringAtColumnNumber(FString  InputData, FString& OutData, int32 ColumnNumber);


	/**
	* ReplaceStringAtColumnNumber - returns a string at a specific column number by StringToReplace
	* @param InputData the string to whch changes are to be made
	* @param StringToReplace string to be replaced
	* @param OutData data is returned as a reference
	* @param ColumnNumber
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|File")
		void ReplaceStringAtColumnNumber(FString  InputData, FString StringToReplace, FString& OutData, int32 ColumnNumber);


	/**
	* ReplaceStringAtLineNumber - Draws the line divider between the sections.
	* @param InputData currently takes file content can be re written for multiple files
	* @param StringToReplace string to be replaced
	* @param OutData data is returned as a reference
	* @param LineNumber
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|GameMode|File")
		void ReplaceStringAtLineNumber(FString  InputData, FString StringToReplace, FString& OutData, int32 LineNumber);

	UFUNCTION(BlueprintCallable, Category="EyeX Simulation")
	bool IsEyeXSimulating() const;

protected:
	/** The widget class we will use as our menu when the game starts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UMG Game")
		TSubclassOf<UUserWidget> StartingWidgetClass;

	/** The widget instance that we are using as our menu. */
	UPROPERTY()
		UUserWidget* CurrentWidget;

	/** The content of the file read will be stored here. */
	FString* FileContent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EyeX Simulation")
	bool bSimulateEyeX;

private:
	UEyeXPluginEx* EyeXEx;
	FString PlayerName;

};

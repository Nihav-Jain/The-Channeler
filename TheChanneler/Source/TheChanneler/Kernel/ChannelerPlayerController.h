// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "ChannelerPlayerController.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class THECHANNELER_API AChannelerPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AChannelerPlayerController();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	class UInputDeviceManager* InputDeviceManager;
};

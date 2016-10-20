// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GhostCameraActor.generated.h"

UCLASS()
class THECHANNELER_API AGhostCameraActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGhostCameraActor();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* GhostCam;

private:
	class USceneComponent* mRootComponent;
	class APlayerController* mController;
	class AChannelerCharacter* mChanneler;
};

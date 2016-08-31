// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Characters/NPC.h"
#include "Companion.generated.h"

/**
 * Abstract base class for all Companions
 */
UCLASS(abstract)
class THECHANNELER_API ACompanion : public ANPC
{
	GENERATED_BODY()

public:
	

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Companion")
	FInputActionKeyMapping AttackKey;
	

};

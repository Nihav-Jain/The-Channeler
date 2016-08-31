// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TP_ThirdPerson/TP_ThirdPersonCharacter.h"
#include "../AttackComponents/NPCAttackComponent.h"
#include "NPC.generated.h"

/**
 * Abstract base class for any Non-Player Character in TheChanneler
 */
UCLASS(abstract)
class THECHANNELER_API ANPC : public ATP_ThirdPersonCharacter
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	int32 Health;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	virtual void Attack();

	UNPCAttackComponent* AttackComponent;
};

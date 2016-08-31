// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttackComponents/NPCAttackComponent.h"
#include "ProjectileAttackComponent.generated.h"

/**
 * Actor component to perform projectile based attacks
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THECHANNELER_API UProjectileAttackComponent : public UNPCAttackComponent
{
	GENERATED_BODY()
	
	
public:
	virtual void PerformAttack() override;
	
};

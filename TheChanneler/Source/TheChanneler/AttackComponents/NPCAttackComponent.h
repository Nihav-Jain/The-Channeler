// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "NPCAttackComponent.generated.h"


UCLASS(abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THECHANNELER_API UNPCAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNPCAttackComponent();

	virtual void BeginPlay() override;
	
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	virtual void PerformAttack();
	
};

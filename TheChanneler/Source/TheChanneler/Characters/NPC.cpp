// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "../AttackComponents/NPCAttackComponent.h"
#include "NPC.h"



void ANPC::BeginPlay()
{
	Super::BeginPlay();
	TArray<UNPCAttackComponent*> attackComponents;
	GetComponents(attackComponents);

	UE_LOG(LogTemp, Warning, TEXT("ANPC::BeginPlay()"));

	if (attackComponents.Num() > 0)
	{
		AttackComponent = attackComponents[0];
	}
	else
	{
		AttackComponent = nullptr;
		UE_LOG(LogTemp, Error, TEXT("No Attack component attached"));
	}
}

void ANPC::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("ANPC::Attack()"));

	if (AttackComponent != nullptr)
	{
		AttackComponent->PerformAttack();
	}
}



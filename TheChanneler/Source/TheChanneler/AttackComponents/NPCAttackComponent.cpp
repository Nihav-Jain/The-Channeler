// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "NPCAttackComponent.h"


UNPCAttackComponent::UNPCAttackComponent()
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}


void UNPCAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UNPCAttackComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UNPCAttackComponent::PerformAttack()
{
	UE_LOG(LogTemp, Log, TEXT("UNPCAttackComponent::PerformAttack()"));
}

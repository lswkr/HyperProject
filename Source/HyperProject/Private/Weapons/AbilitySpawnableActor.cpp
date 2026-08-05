// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/AbilitySpawnableActor.h"

// Sets default values
AAbilitySpawnableActor::AAbilitySpawnableActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void AAbilitySpawnableActor::BeginPlay()
{
	Super::BeginPlay();
	
}


void AAbilitySpawnableActor::SetAbilitySystem(UAbilitySystemComponent* InASC)
{
	AbilitySystemComponent = InASC;
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/AbilitySpawnableActor.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AAbilitySpawnableActor::AAbilitySpawnableActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void AAbilitySpawnableActor::SetGenericTeamId(const FGenericTeamId& InTeamId)
{
	TeamID = InTeamId;
}

void AAbilitySpawnableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAbilitySpawnableActor, TeamID);
}

void AAbilitySpawnableActor::BeginPlay()
{
	Super::BeginPlay();
	
}


void AAbilitySpawnableActor::SetAbilitySystem(UAbilitySystemComponent* InASC)
{
	AbilitySystemComponent = InASC;
}


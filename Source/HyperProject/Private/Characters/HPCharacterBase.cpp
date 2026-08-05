// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/HPCharacterBase.h"

#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Controller/HPPlayerController.h"

// Sets default values
AHPCharacterBase::AHPCharacterBase()
{
 	PrimaryActorTick.bCanEverTick = true;

	HPAbilitySystemComponent = CreateDefaultSubobject<UHPAbilitySystemComponent>("HPAbilitySystemComponent");
	HPAttributeSet = CreateDefaultSubobject<UHPAttributeSet>("HPAttributeSet");
}

void AHPCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHPCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHPCharacterBase::ServerSideInit()
{
	HPAbilitySystemComponent->InitAbilityActorInfo(this,this);
	HPAbilitySystemComponent->ServerSideInit();
}

void AHPCharacterBase::ClientSideInit()
{
	HPAbilitySystemComponent->InitAbilityActorInfo(this,this);
}

UAbilitySystemComponent* AHPCharacterBase::GetAbilitySystemComponent() const
{
	return HPAbilitySystemComponent;
}

UAttributeSet* AHPCharacterBase::GetAttributeSet() const
{
	return HPAttributeSet;
}

UHPCombatComponent* AHPCharacterBase::GetCombatComponent() const
{
	return CombatComponent;
}

void AHPCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();

		if (AHPPlayerController* HPPC = Cast<AHPPlayerController>(NewController))
		{
			HPPC->HighPingDelegate
		}
	}
}




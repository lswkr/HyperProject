// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/HPPlayerCharacter_UsingDC.h"

#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "Components/DetectComponent.h"
#include "Controller/HPPlayerController.h"

AHPPlayerCharacter_UsingDC::AHPPlayerCharacter_UsingDC()
{
	DetectComponent = CreateDefaultSubobject<UDetectComponent>("DetectComponent");
	
}

void AHPPlayerCharacter_UsingDC::ClientSideInit()
{
	Super::ClientSideInit();

	if (AHPPlayerController* CurrentHPPlayerController = GetHPPlayerController())
	{
		DetectComponent->InitComponent(HPAbilitySystemComponent, CurrentHPPlayerController, this);
	}
	
}

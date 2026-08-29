// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HPAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/HPHUD.h"
#include "UI/WidgetController/HPWidgetController.h"

bool UHPAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject,
                                                         FWidgetControllerParams& OutWidgetControllerParams, AHPHUD*& OutHPHUD)
{
	if (APlayerController * PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutHPHUD = Cast<AHPHUD>(PC->GetHUD());

		if (OutHPHUD)
		{
			AHPPlayerCharacter* HPPawn = Cast<AHPPlayerCharacter>(PC->GetPawn());

			UAbilitySystemComponent* ASC = HPPawn->GetAbilitySystemComponent();
			UAttributeSet* AS = HPPawn->GetAttributeSet();
			UHPCombatComponent* CC = HPPawn->GetCombatComponent();

			if (ASC && AS && CC)
			{
				OutWidgetControllerParams.PlayerController = PC;
				OutWidgetControllerParams.AbilitySystemComponent = ASC;
				OutWidgetControllerParams.AttributeSet = AS;
				OutWidgetControllerParams.CombatComponent = CC;
				return true;
			}
		}
	}
	return false;
}

UHPCombatComponent* UHPAbilitySystemLibrary::GetCombatComponent(const AActor* InActor)
{
	if (const AHPCharacterBase* HPCharacter = Cast<AHPCharacterBase>(InActor))
	{
		return HPCharacter->GetCombatComponent();
	}
	return nullptr;
}

TArray<AHPPlayerCharacter*> UHPAbilitySystemLibrary::GetSameTeamCharactersToIgnore(const UObject* WorldContextObject,
	AHPPlayerCharacter* CurrentPlayer)
{
	UWorld* World = WorldContextObject->GetWorld();
	TArray<AHPPlayerCharacter*> ResultArray = TArray<AHPPlayerCharacter*>();
	
	if (!World)
		return ResultArray;
	
	for (TActorIterator<AHPPlayerCharacter> It(World); It; ++It)
	{
		AHPPlayerCharacter* IteratedCharacter = *It;

		if (!IteratedCharacter)
			continue;

		if (CurrentPlayer->GetGenericTeamId() == IteratedCharacter->GetGenericTeamId())
		{
			ResultArray.Add(IteratedCharacter);
		}
	}
	return ResultArray;
}

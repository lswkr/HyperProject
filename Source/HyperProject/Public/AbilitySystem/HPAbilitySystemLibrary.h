// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HPAbilitySystemLibrary.generated.h"

/**
 * 
 */
class AHPHUD;
class UHPCombatComponent;

struct FWidgetControllerParams;

UCLASS()
class HYPERPROJECT_API UHPAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWidgetControllerParams, AHPHUD*& OutHPHUD);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	static UHPCombatComponent* GetCombatComponent(const AActor* InActor);

	UFUNCTION(BlueprintPure, Category="Combat", meta = (DefaultToSelf = "WorldContextObject"))
	static TArray<AHPPlayerCharacter*> GetSameTeamCharactersToIgnore(const UObject* WorldContextObject, AHPPlayerCharacter* CurrentPlayer); //상대
};

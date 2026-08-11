// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HPNetStatics.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPNetStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static uint8 GetPlayerCountPerTeam();
};

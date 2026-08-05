// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "HPAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UHPAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};

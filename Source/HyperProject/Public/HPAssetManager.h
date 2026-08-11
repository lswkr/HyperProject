// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "HPAssetManager.generated.h"

/**
 * 
 */
class UPDA_CharacterDefinition;

UCLASS()
class HYPERPROJECT_API UHPAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UHPAssetManager& Get();
	void  LoadCharacterDefinitions(const FStreamableDelegate& LoadFinishedCallback);
	bool GetLoadedCharacterDefinitions(TArray<UPDA_CharacterDefinition*>& LoadedCharacterDefinitions) const;
protected:
	virtual void StartInitialLoading() override;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "HPAssetManager.h"

#include "HPGameplayTags.h"
#include "Characters/PDA_CharacterDefinition.h"

UHPAssetManager& UHPAssetManager::Get()
{
	check(GEngine);

	UHPAssetManager* HPAssetManager = Cast<UHPAssetManager>(GEngine->AssetManager);
	return *HPAssetManager;
}

void UHPAssetManager::LoadCharacterDefinitions(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPDA_CharacterDefinition::GetCharacterDefinitionAssetType(), TArray<FName>(), LoadFinishedCallback);
}

bool UHPAssetManager::GetLoadedCharacterDefinitions(TArray<UPDA_CharacterDefinition*>& LoadedCharacterDefinitions) const
{
	TArray<UObject*> LoadedObjects;
	bool bLoaded = GetPrimaryAssetObjectList(UPDA_CharacterDefinition::GetCharacterDefinitionAssetType(), LoadedObjects);

	if (bLoaded)
	{
		for (UObject* LoadedObject : LoadedObjects)
		{
			LoadedCharacterDefinitions.Add(Cast<UPDA_CharacterDefinition>(LoadedObject));
		}
	}

	return bLoaded;
}

void UHPAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FHPGameplayTags::InitializeNativeGameplayTags();
}

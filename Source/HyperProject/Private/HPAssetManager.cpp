// Fill out your copyright notice in the Description page of Project Settings.


#include "HPAssetManager.h"

#include "HPGameplayTags.h"

UHPAssetManager& UHPAssetManager::Get()
{
	check(GEngine);

	UHPAssetManager* HPAssetManager = Cast<UHPAssetManager>(GEngine->AssetManager);
	return *HPAssetManager;
}

void UHPAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FHPGameplayTags::InitializeNativeGameplayTags();
}

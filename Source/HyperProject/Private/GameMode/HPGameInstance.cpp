// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HPGameInstance.h"

void UHPGameInstance::StartMatch()
{
	if (GetWorld()->GetNetMode()==ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		LoadLevelAndListen(GameLevel);
		return;
	}
	
}

void UHPGameInstance::LoadLevelAndListen(TSoftObjectPtr<UWorld> Level)
{
	const FName LevelURL = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));

	if (LevelURL != "")
	{
		GetWorld()->ServerTravel(LevelURL.ToString() + "?listen");
	}
}

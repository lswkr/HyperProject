// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/HPGameSession.h"

#include "GameMode/HPGameInstance.h"

bool AHPGameSession::ProcessAutoLogin()
{
	return true;
}

void AHPGameSession::RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite)
{
	Super::RegisterPlayer(NewPlayer, UniqueId, bWasFromInvite);

	if (UHPGameInstance* GameInstance = GetGameInstance<UHPGameInstance>())
	{
		GameInstance->PlayerJoined(UniqueId);
	}
}

void AHPGameSession::UnregisterPlayer(FName InSessionName, const FUniqueNetIdRepl& UniqueId)
{
	Super::UnregisterPlayer(InSessionName, UniqueId);

	if (UHPGameInstance* GameInstance = GetGameInstance<UHPGameInstance>())
	{
		GameInstance->PlayerLeft(UniqueId);
	}
}

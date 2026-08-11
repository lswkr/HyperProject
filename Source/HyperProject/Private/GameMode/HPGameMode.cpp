// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HPGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "PlayerState/HPPlayerState.h"


APlayerController* AHPGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId TeamId = GetTeamIDForPlayer(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}

	NewPlayerController->StartSpot = FindNextStartSpotForTeam(TeamId);
	return NewPlayerController;
}

UClass* AHPGameMode::GetDefaultPawnClassForController_Implementation(AController* Controller)
{
	AHPPlayerState* HPPlayerState = Controller->GetPlayerState<AHPPlayerState>();

	if (HPPlayerState && HPPlayerState->GetSelectedCharacterClass())
	{
		return HPPlayerState->GetSelectedCharacterClass();
	}
	return BackupPawn;
}

APawn* AHPGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayer);

	FGenericTeamId TeamId = GetTeamIDForPlayer(NewPlayer);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}

	StartSpot = FindNextStartSpotForTeam(TeamId);
	NewPlayer->StartSpot = StartSpot;
	
	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

void AHPGameMode::StartPlay()
{
	Super::StartPlay();
}

FGenericTeamId AHPGameMode::GetTeamIDForPlayer(const AController* InController) const
{
	AHPPlayerState* HPPlayerState = InController->GetPlayerState<AHPPlayerState>();

	if (HPPlayerState && HPPlayerState->GetSelectedCharacterClass())
	{
		return HPPlayerState->GetTeamIdBasedOnSlot();
	}
	
	static int PlayerCount = 0;
	++PlayerCount;
	return FGenericTeamId(PlayerCount % 2);
}

AActor* AHPGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);
	if (!StartSpotTag)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}
	return nullptr;
}

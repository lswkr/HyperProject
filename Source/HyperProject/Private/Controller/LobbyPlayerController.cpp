// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/LobbyPlayerController.h"
#include "GameMode/HPGameState.h"
#include "GameFramework/PlayerState.h"
#include "GameMode/HPGameInstance.h"

void ALobbyPlayerController::Server_RequestSlotSelectionChange_Implementation(uint8 NewSlotID)
{
	if (!GetWorld()) return;

	AHPGameState* HPGameState = GetWorld()->GetGameState<AHPGameState>();
	if (!HPGameState)
		return;

	HPGameState->RequestPlayerSelectionChange(GetPlayerState<APlayerState>(), NewSlotID);
}

bool ALobbyPlayerController::Server_RequestSlotSelectionChange_Validate(uint8 NewSlotID)
{
	return true;
}

void ALobbyPlayerController::Server_StartHeroSelection_Implementation()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator PlayerControllerIterator = GetWorld()->GetPlayerControllerIterator(); PlayerControllerIterator; ++PlayerControllerIterator)
	{
		ALobbyPlayerController* PlayerController = Cast<ALobbyPlayerController>(*PlayerControllerIterator);

		if (PlayerController)
		{
			PlayerController->Client_StartHeroSelection();
		}
	}
}

bool ALobbyPlayerController::Server_StartHeroSelection_Validate()
{
	return true;	
}

void ALobbyPlayerController::Client_StartHeroSelection_Implementation()
{
	OnSwitchToHeroSelectionDelegate.ExecuteIfBound();
}

ALobbyPlayerController::ALobbyPlayerController()
{
	bAutoManageActiveCameraTarget = false; //바로 카메라에 캐릭터가 잡히지 않도록
}

void ALobbyPlayerController::Server_RequestStartMatch_Implementation()
{
	UHPGameInstance* HPGameInstance = GetWorld()->GetGameInstance<UHPGameInstance>();

	if (HPGameInstance)
	{
		HPGameInstance->StartMatch();
	}
}

bool ALobbyPlayerController::Server_RequestStartMatch_Validate()
{
	return true;
}

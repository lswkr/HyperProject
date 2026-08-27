// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/MenuPlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 * 
 */
DECLARE_DELEGATE(FOnSwitchToHeroSelectionDelegate);

UCLASS()
class HYPERPROJECT_API ALobbyPlayerController : public AMenuPlayerController
{
	GENERATED_BODY()

public:
	ALobbyPlayerController();
	
	FOnSwitchToHeroSelectionDelegate OnSwitchToHeroSelectionDelegate;
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestSlotSelectionChange(uint8 NewSlotID);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartHeroSelection();

	UFUNCTION(Client, Reliable)
	void Client_StartHeroSelection();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestStartMatch();

	FORCEINLINE uint8 GetSelectedSlotId() const {return SelectedSlotID; }
	
private:
	UFUNCTION(Client,Reliable)
	void Client_SetSelectionSlot(uint8 NewSlotID);

	uint8 SelectedSlotID = -1;

	
};

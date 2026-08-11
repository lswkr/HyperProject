// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.h"
#include "GameFramework/GameStateBase.h"
#include "HPGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSelectionUpdatedDelegate, const TArray<FPlayerSelection>& /*NewPlayerSelection*/)
/**
 * 
 */
UCLASS()
class HYPERPROJECT_API AHPGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void RequestPlayerSelectionChange(const APlayerState* RequestingPlayer, uint8 DesiredSlot);
	bool IsSlotOccupied(uint8 SlotId) const;

	FOnPlayerSelectionUpdatedDelegate OnPlayerSelectionUpdatedDelegate;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<FPlayerSelection>& GetPlayerSelection() const;
	
private:
	UPROPERTY(Replicated = OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;

	UFUNCTION()
	void  OnRep_PlayerSelectionArray();
	
};

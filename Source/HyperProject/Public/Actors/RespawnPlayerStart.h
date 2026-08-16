// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "RespawnPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API ARespawnPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	int32 GetPlayerStartControlPointNum() const { return ControlPointNum; }
	bool IsOccupied() const { return bIsOccupied; }
	void SetOccupied(bool InIsOccupied) { bIsOccupied =InIsOccupied; }

private:
	UPROPERTY(EditAnywhere, Category = "RespawnPlayerStart")
	int32 ControlPointNum;

	UPROPERTY(EditAnywhere, Category = "RespawnPlayerStart")
	int32 PlayerStartsIdx;

	UPROPERTY(EditAnywhere, Category = "RespawnPlayerStart")
	bool bIsOccupied = false;
};

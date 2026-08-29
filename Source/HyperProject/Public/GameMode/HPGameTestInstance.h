// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HPGameInstance.h"
#include "Engine/GameInstance.h"
#include "HPGameTestInstance.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPGameTestInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	void StartMatch();

private:	
	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> LobbyLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> GameLevel;

	void LoadLevelAndListen(TSoftObjectPtr<UWorld> Level);
	
};

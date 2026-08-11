// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HPGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	void StartMatch();
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> MainMenuLevel; //전체를 불러오지 않기 위해 SoftObjectPtr

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> LobbyMenuLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> GameLevel;

	void LoadLevelAndListen(TSoftObjectPtr<UWorld> Level);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ControlPointGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "ControlPointTestGameMode.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API AControlPointTestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	
	public:

	virtual void StartPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	UPROPERTY(EditDefaultsOnly,Category = "ControlPointGameMode")
	TSubclassOf<AHPControlPoint> ControlPointClass;
	
	UPROPERTY()
	TArray<AHPControlPoint*> ControlPoints;

	FTimerHandle ControlPointBindTimerHandle;

	void BindControllerToControlPoints();
	void OnGameStart();
	void BeforeGameLeftTimeCheck();
	void BeforePointActivateLeftTimeCheck();

	int32 CurrentTeam1Point = 0;
	int32 CurrentTeam2Point = 0;
	UPROPERTY(EditDefaultsOnly,Category = "ControlPointGameMode")
	int32 VictoryPoint = 2;
	
	UFUNCTION()
	void OnControlPointCompleted(EControlPointType CompletedControlPoint, int32 TeamID);

	FTimerHandle BeforeGameStartTimerHandle;
	FTimerHandle WaitToTurnOnNextPointTimerHandle; // 거점 켜질 때까지 남은 시간. 
	FTimerHandle SendTimeLeftTimerHandle; //0.25초 주기로 NextPointTimerHandle의 남은시간을 확인 후 시간을 클라에 전송(Tick켜기 아까워서 타이머로 함)

	UPROPERTY(EditDefaultsOnly,Category = "ControlPointGameMode")
	float ActivateNextPointTime = 10.f;

	void TurnOnWaitToTurnOnNextPointTimerHandle(float TimerTime);
	void AfterWaitingTimeToActivateNextPoint();
	/*
	 * StartPlay했을 때 BeforeGameStartTimerHandle
	 * -> BeforeGameStartTimerHandle 끝나면 호출되는 콜백함수에 WaitToTurnOn~~타이머 켜기
	 * -> 거점 활성화까지 남은시간 표시하는 GameState Rep변수
	 * -> 
	 */

	UPROPERTY()
	AControlPointGameState* CPGameState;
	
	int32 CurrentTargetPointIdx = 0;

	EControlPointGameModeState ControlPointGameModeState = EControlPointGameModeState::GameComplete;


	/*
	 * HPGameMode Begin
	 */

public:
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

	ARespawnPlayerStart* GetRespawnPlayerStart(AHPPlayerCharacter* RespawningCharacter);
private:
	FGenericTeamId GetTeamIDForPlayer(const APlayerController* PlayerController) const;

	AActor* FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;

	UPROPERTY()
	TArray<ARespawnPlayerStart*> RespawnPlayerStarts; 
	/*
	 * HPGameMode End
	 */

	
};

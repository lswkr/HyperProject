// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "UI/HUD/HPControlPointHUD.h"
#include "ControlPointGameState.generated.h"

class AHPPlayerController;
enum class EControlPointGameModeState: uint8;


DECLARE_DYNAMIC_DELEGATE_TwoParams(FGameStateIntValueDelegate_TwoParams,int32, MyTeamValue, int32, EnemyTeamValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGameStateIntValueDelegate_OneParam,int32, IntValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGameStateStringValueDelegate,FString, StringValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGameModeStateChangeDelegate, EControlPointGameModeState, GameModeValue);

/**
 * 
 */

UCLASS()
class HYPERPROJECT_API AControlPointGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void OnTeamCompletePoint(int32 Team1Point, int32 Team2Point, int32 InNextPoint);
	void SetTimeCount(int32 InTimeRemaining);
	void SetControlPointGameModeState(EControlPointGameModeState InControlPointGameModeState);
	
	//NEXTTHINGTODO: 게임 모드 거점활성화, 게임 시작 전 표시하는 위젯 연결하도록 하나 뭐 만들기
	UFUNCTION()
	void OnRep_TimeRemaining();

	UFUNCTION()
	void OnRep_TeamOneCount();

	UFUNCTION()
	void OnRep_TeamTwoCount();

	UFUNCTION()
	void OnRep_NextPoint();

	UFUNCTION()
	void OnRep_ControlPointModeChanged();

	UFUNCTION()
	void OnRep_TeamOneCaptured();

	UFUNCTION()
	void OnRep_TeamTwoCaptured();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void BindHUD(AHPControlPointHUD* InHUD);

private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamOneCount)
	int32 TeamOnePoint = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TeamTwoCount)
	int32 TeamTwoPoint = 0; 

	UPROPERTY(ReplicatedUsing = OnRep_TimeRemaining)
	int32 TimeRemaining;

	UPROPERTY(ReplicatedUsing = OnRep_TeamOneCaptured)
	bool TeamOneCaptured = false;

	UPROPERTY(ReplicatedUsing = OnRep_TeamTwoCaptured)
	bool TeamTwoCaptured = false;

	UPROPERTY(ReplicatedUsing = OnRep_ControlPointModeChanged)
	EControlPointGameModeState CurrentControlPointGameModeState; 
	
	UPROPERTY()
	AHPPlayerController* LocalPlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_NextPoint)
	FString NextPoint;

	FGameStateIntValueDelegate_TwoParams TeamCountDelegate;
	FGameStateIntValueDelegate_OneParam TimeRemainingDelegate;
	FGameModeStateChangeDelegate GameModeStateChangeDelegate;
};

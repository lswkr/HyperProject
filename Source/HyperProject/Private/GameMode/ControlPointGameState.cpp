// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ControlPointGameState.h"

#include "Controller/HPPlayerController.h"
#include "GameFramework/GameSession.h"
#include "Net/UnrealNetwork.h"

void AControlPointGameState::OnTeamCompletePoint(int32 Team1Point, int32 Team2Point, int32 InNextPoint)
{
	TeamOnePoint=Team1Point;
	TeamTwoPoint=Team2Point;

	NextPoint = FString::Chr(TEXT('A') + InNextPoint);
}

void AControlPointGameState::SetTimeCount(int32 InTimeRemaining)
{
	TimeRemaining = InTimeRemaining;
}

void AControlPointGameState::SetControlPointGameModeState(EControlPointGameModeState InControlPointGameModeState)
{
	CurrentControlPointGameModeState = InControlPointGameModeState;
}

void AControlPointGameState::OnRep_TimeRemaining()
{
	TimeRemainingDelegate.ExecuteIfBound(TimeRemaining);	
}

void AControlPointGameState::OnRep_TeamOneCount()
{
	TeamCountDelegate.ExecuteIfBound(TeamOnePoint,TeamTwoPoint);
}

void AControlPointGameState::OnRep_TeamTwoCount()
{
	TeamCountDelegate.ExecuteIfBound(TeamOnePoint,TeamTwoPoint);
}

void AControlPointGameState::OnRep_NextPoint()
{
	LocalPlayerController = LocalPlayerController== nullptr ? Cast<AHPPlayerController>(GetWorld()->GetFirstPlayerController()):LocalPlayerController;
}

void AControlPointGameState::OnRep_ControlPointModeChanged()
{
	GameModeStateChangeDelegate.ExecuteIfBound(CurrentControlPointGameModeState);
}

void AControlPointGameState::OnRep_TeamOneCaptured()
{
	if (TeamOneCaptured)
	{
		TeamTwoCaptured = false;
	}
}

void AControlPointGameState::OnRep_TeamTwoCaptured()
{
	if (TeamTwoCaptured)
	{
		TeamOneCaptured = false;
	}
}

void AControlPointGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlPointGameState, TeamOnePoint, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlPointGameState, TeamTwoPoint, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlPointGameState, TimeRemaining, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlPointGameState, NextPoint, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlPointGameState, CurrentControlPointGameModeState, COND_None, REPNOTIFY_Always);
}


void AControlPointGameState::BindHUD(AHPControlPointHUD* InHUD)
{
	if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController() )
		if (LocalPC->IsLocalController())
		{
			TeamCountDelegate.BindDynamic(InHUD, &AHPControlPointHUD::OnUpdateCapturePointCount);
			TimeRemainingDelegate.BindDynamic(InHUD, &AHPControlPointHUD::OnUpdateTimeRemaining);
			GameModeStateChangeDelegate.BindDynamic(InHUD, &AHPControlPointHUD::OnUpdateControlPointGameModeState);
		}
}

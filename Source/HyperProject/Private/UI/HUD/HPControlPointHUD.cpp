// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/HPControlPointHUD.h"

#include "Controller/HPPlayerController.h"
#include "GameMode/ControlPointGameState.h"
#include "UI/Widget/HPControlPointOverlayWidget.h"

void AHPControlPointHUD::InitOverlay(APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS,
                                     UHPCombatComponent* CC)
{
	Super::InitOverlay(PC, ASC, AS, CC);

	HPPlayerController = Cast<AHPPlayerController>(PC);
	ControlPointGameState = Cast<AControlPointGameState>(GetWorld()->GetGameState());

	HPPlayerController->BindControlPointGameModeHUD(this);
	
	ControlPointOverlayWidget = Cast<UHPControlPointOverlayWidget>(OverlayWidget);

	//GameState nullptr방지
	UWorld* World = GetWorld();
	if (!World)
		return;

	if (AControlPointGameState* CPGameState =
		Cast<AControlPointGameState>(World->GetGameState()))
	{
		CPGameState->BindHUD(this);
	}
	else
	{
		World->GameStateSetEvent.AddLambda([this](AGameStateBase* GSBase)
		{
			if (AControlPointGameState* CPGameState = Cast<AControlPointGameState>(GSBase))
			{
				CPGameState->BindHUD(this);
			}
		});
	}
}

void AHPControlPointHUD::OnUpdateControlPoint(float MyTeamFightingPercent, float EnemyTeamFightingPercent,
	float MyTeamCapturingPercent, float EnemyTeamTwoCapturingPercent) 
{
	ControlPointOverlayWidget->UpdateProgressBar(MyTeamFightingPercent, EnemyTeamFightingPercent,MyTeamCapturingPercent,EnemyTeamTwoCapturingPercent);
}

void AHPControlPointHUD::OnUpdateTimeRemaining(int32 RemainingTime) 
{
	ControlPointOverlayWidget->FloatTimeRemaining(RemainingTime);
}

void AHPControlPointHUD::OnUpdateCapturePointCount(int32 TeamOneCount, int32 TeamTwoCount) 
{
	int32 PlayerTeamID = HPPlayerController->GetGenericTeamId().GetId();

	int32 MyTeamCount = 0;
	int32 EnemyTeamCount = 0;
	
	if (PlayerTeamID == 0)
	{
		MyTeamCount = TeamOneCount;
		EnemyTeamCount = TeamTwoCount;
	}
	else if (PlayerTeamID == 1)
	{
		MyTeamCount = TeamTwoCount;
		EnemyTeamCount = TeamOneCount;
	}

	ControlPointOverlayWidget->UpdateTeamCompletePoint(MyTeamCount, EnemyTeamCount);
}

void AHPControlPointHUD::OnUpdateControlPointGameModeState(EControlPointGameModeState CurrentControlPointGameModeState)
{
	ControlPointOverlayWidget->UpdateControlPointGameModeState(CurrentControlPointGameModeState);
}


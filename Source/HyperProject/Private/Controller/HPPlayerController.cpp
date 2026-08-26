// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/HPPlayerController.h"

#include "Actors/HPControlPoint.h"
#include "Blueprint/UserWidget.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/HPCombatComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "UI/HUD/HPControlPointHUD.h"
#include "UI/HUD/HPHUD.h"
#include "UI/Widget/MatchResultWidget.h"

void AHPPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckPing(DeltaTime);
	CheckTimeSync(DeltaTime);
}

void AHPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	HPPlayerCharacter = Cast<AHPPlayerCharacter>(InPawn);

	if (HPPlayerCharacter)
	{
		HPPlayerCharacter->ServerSideInit();
		HPPlayerCharacter->SetGenericTeamId(TeamID);
		
		HighPingDelegate.AddDynamic(HPPlayerCharacter->GetCombatComponent(), &UHPCombatComponent::ShouldUseServerSideRewind);
	}
}

void AHPPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);

	HPPlayerCharacter = Cast<AHPPlayerCharacter>(InPawn);

	if (HPPlayerCharacter)
	{
		HPPlayerCharacter->ClientSideInit();	
	}
}

void AHPPlayerController::PlayHitFeedbackWidget(bool bIsHeadShot)
{
	if (!HPHUD)
	{
		HPHUD = Cast<AHPHUD>(GetHUD());
	}
	if (HPHUD)
	{
		HPHUD->PlayHitFeedback(bIsHeadShot);
	}
}

void AHPPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHPPlayerController, TeamID);
}

FGenericTeamId AHPPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void AHPPlayerController::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	TeamID = InTeamID;
}

void AHPPlayerController::UpdateControlPointState(const FHPControlPointData& ControlPointData)
{
	float MyTeamFightingPercent = 0.f;
	float EnemyTeamFightingPercent = 0.f;
	float MyTeamCapturingPercent = 0.f;
	float EnemyTeamCapturingPercent = 0.f;
	
	if (TeamID == 0)
	{
		MyTeamFightingPercent = ControlPointData.TeamOneFightingPercent;
		EnemyTeamFightingPercent  = ControlPointData.TeamTwoFightingPercent;
		
		MyTeamCapturingPercent = ControlPointData.TeamOneCapturingPercent;
		EnemyTeamCapturingPercent = ControlPointData.TeamTwoCapturingPercent;
	}
	else if (TeamID == 1)
	{
		MyTeamFightingPercent = ControlPointData.TeamTwoFightingPercent;
		EnemyTeamFightingPercent  = ControlPointData.TeamOneFightingPercent;

		MyTeamCapturingPercent = ControlPointData.TeamTwoCapturingPercent;
		EnemyTeamCapturingPercent = ControlPointData.TeamOneCapturingPercent;
	}

	//브로드캐스트
	CPHUD->OnUpdateControlPoint(MyTeamFightingPercent, EnemyTeamFightingPercent, MyTeamCapturingPercent, EnemyTeamCapturingPercent);
}

void AHPPlayerController::UpdateCapturedTeamState(bool TeamOneCaptured, bool TeamTwoCaptured)
{
	if (TeamID == 0)
	{
		UE_LOG(LogTemp, Warning,TEXT("UpdateCapturedTeamState: MyTeam: %d, EnemyTeam %d"), TeamOneCaptured, TeamTwoCaptured);
		CPHUD->OnControlPointCaptured(TeamOneCaptured, TeamTwoCaptured);
	}
	else if (TeamID == 1)
	{
		UE_LOG(LogTemp, Warning,TEXT("UpdateCapturedTeamState: MyTeam: %d, EnemyTeam %d"), TeamTwoCaptured, TeamOneCaptured);
		CPHUD->OnControlPointCaptured(TeamTwoCaptured, TeamOneCaptured);
	}
}

void AHPPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	UE_LOG(LogTemp, Warning, TEXT("bHighPing = %d"), bHighPing);
	HighPingDelegate.Broadcast(bHighPing);
}

void AHPPlayerController::HighPingWarning()
{
	//NEXTTHINGTODO
}

void AHPPlayerController::StopHighPingWarning()
{
	//NEXTTHINGTODO
}

void AHPPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		if (PlayerState == nullptr)
		{
			PlayerState =  GetPlayerState<APlayerState>();
		}
		if (PlayerState)
		{
			if (PlayerState -> GetCompressedPing() * 4 > HighPingThreshold)
			{
				//NEXTTHINGTODO: high ping Warning
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	/*
	 * if(애니메이션 플레이 중인가)
	 * {
	 *		AnimationRunningTime += DeltaTime;
	 *		if(PingAnimationRunningTime > HighPingDuration)
	 *		{
	 *			HighpingWarning애니메이션 멈추기
	 *		}
	 * }
	 * 
	 */
}

void AHPPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AHPPlayerController::Client_ShowResultWidget_Implementation(uint8 TeamNum)
{
	if (TeamNum == GetGenericTeamId().GetId())
	{
		UUserWidget* VictoryWidget = CreateWidget<UUserWidget> (this, VictoryWidgetClass);
		//VictoryWidget->SetResult();
		VictoryWidget->AddToViewport();		
	}
	else
	{
		UUserWidget* DefeatWidget = CreateWidget<UUserWidget> (this, DefeatWidgetClass);
		//DefeatWidget->SetResult();
		DefeatWidget->AddToViewport();
	}
}

void AHPPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AHPPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AHPPlayerController::BindControlPointGameModeHUD(AHPControlPointHUD* InCPHUD)
{
	CPHUD = InCPHUD;
	OnUpdateControlPointDelegate.BindDynamic(CPHUD, &AHPControlPointHUD::OnUpdateControlPoint);
}

float AHPPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AHPPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}


void AHPPlayerController::Client_BindControlPoints_Implementation(const TArray<AHPControlPoint*>& StageControlPoints)
{
	for (AHPControlPoint* ControlPoint : StageControlPoints)
	{
		if (!ControlPoint)
			continue;

		ControlPoint->BindPlayerControllerToControlPoint(this);
	}
}

void AHPPlayerController::Client_OnReceiveEnemyEliminationInfo_Implementation(FName PlayerNickname,
                                                                              float ContributionPercent)
{
	AHPHUD* HUD = Cast<AHPHUD>(GetHUD());

	if (!HUD)
		return;

	HUD->GetOverlayWidgetFloatKillLog(PlayerNickname, ContributionPercent*100);
}

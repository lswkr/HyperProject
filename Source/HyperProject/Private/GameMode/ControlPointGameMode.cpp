// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ControlPointGameMode.h"

#include "EngineUtils.h"
#include "Actors/ControlPointSpawnTargetPoint.h"
#include "Actors/HPControlPoint.h"
#include "Actors/RespawnPlayerStart.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Controller/HPPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameMode/ControlPointGameState.h"
#include "Kismet/GameplayStatics.h"

APlayerController* AControlPointGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	TArray<AActor*> CurrentPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(
	this,
	ARespawnPlayerStart::StaticClass(),
	CurrentPlayerStarts);

	for (AActor* PlayerStartActor : CurrentPlayerStarts)
	{
		if (ARespawnPlayerStart* RespawnPlayerStart = Cast<ARespawnPlayerStart>(PlayerStartActor))
		{
			RespawnPlayerStarts.Add(RespawnPlayerStart);
		}
	}

	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId TeamId = GetTeamIDForPlayer(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}

	NewPlayerController->StartSpot = FindNextStartSpotForTeam(TeamId);
	return NewPlayerController;
}

ARespawnPlayerStart* AControlPointGameMode::GetRespawnPlayerStart(AHPPlayerCharacter* RespawningCharacter)
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(RespawningCharacter->GetGenericTeamId());

	UE_LOG(LogTemp, Warning,TEXT("RespawningCharacter GenricId %d"), RespawningCharacter->GetGenericTeamId().GetId());
	if (!StartSpotTag)
	{
		return nullptr;
	}
	
	for (ARespawnPlayerStart* PlayerStart : RespawnPlayerStarts)
	{
		if (PlayerStart->PlayerStartTag == *StartSpotTag &&
			PlayerStart->GetPlayerStartControlPointNum() == CurrentTargetPointIdx &&
			!PlayerStart->IsOccupied())
		{
			PlayerStart->SetOccupied(true);
			return PlayerStart;
		}
	}
	return nullptr;
}

void AControlPointGameMode::StartPlay()
{
	Super::StartPlay();
	ControlPoints.SetNum(3);

	CPGameState = GetGameState<AControlPointGameState>();
	
	TArray<AActor*> FoundTargetPoints;
	
	UGameplayStatics::GetAllActorsOfClass(
		this,
		AControlPointSpawnTargetPoint::StaticClass(),
		FoundTargetPoints
		);

	for (AActor* FoundTargetPoint : FoundTargetPoints)
	{
		AControlPointSpawnTargetPoint* ControlPointTargetPoint = Cast<AControlPointSpawnTargetPoint>(FoundTargetPoint);

		int32 idx = static_cast<int32> (ControlPointTargetPoint->TargetPointType);
		AHPControlPoint* ControlPoint = GetWorld()->SpawnActor<AHPControlPoint>(
			ControlPointClass,
			ControlPointTargetPoint->GetActorTransform()
			);
		ControlPoint->SetControlPointState(ControlPointTargetPoint->TargetPointType);
		ControlPoint->ActivateControlPoint(false);
		ControlPoints[idx] = ControlPoint;
		ControlPoint->ControlPointCompletedDelegate.BindDynamic(this, &AControlPointGameMode::OnControlPointCompleted);
	}

	//게임 시작 전 상태 
	ControlPointGameModeState = EControlPointGameModeState::BeforeGameStart;
	CPGameState->SetControlPointGameModeState(ControlPointGameModeState);

	//10초 뒤 게임 시작
	GetWorldTimerManager().SetTimer(
		BeforeGameStartTimerHandle,
		this,
		&AControlPointGameMode::OnGameStart,
		10.f,
		false);

	//게임 시작 전 타이머
	GetWorldTimerManager().SetTimer(
	SendTimeLeftTimerHandle,
	this,
	&AControlPointGameMode::BeforeGameLeftTimeCheck,
	0.25f,
	true
	);

}

void AControlPointGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AHPPlayerController* PC = Cast<AHPPlayerController>(NewPlayer))
	{
		PC->Client_BindControlPoints(ControlPoints);
		//들어왔을 때 GameMode의 상태가 어떤지 알 수 없을 수 있으므로
		CPGameState->SetControlPointGameModeState(ControlPointGameModeState);
	}

}

void AControlPointGameMode::BindControllerToControlPoints()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AHPPlayerController* PC = Cast<AHPPlayerController>(It->Get()))
		{
			PC->Client_BindControlPoints(ControlPoints);
		}
	}
}

void AControlPointGameMode::OnGameStart() //게임 시작(문 열리는 시점)
{
	GetWorldTimerManager().ClearTimer(SendTimeLeftTimerHandle);
	BindControllerToControlPoints();

	TurnOnWaitToTurnOnNextPointTimerHandle(ActivateNextPointTime);
	ControlPointGameModeState = EControlPointGameModeState::WaitToTurnOnNextPoint;
	CPGameState->SetControlPointGameModeState(ControlPointGameModeState);
	
	// ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(true);
	// CPGameState->SetControlPointGameModeState(ControlPointGameModeState);

}

void AControlPointGameMode::BeforeGameLeftTimeCheck()
{
	if (!CPGameState)
	{
		CPGameState=Cast<AControlPointGameState>(GetWorld()->GetGameState());
	}
	float LeftTimeBeforeGame = GetWorldTimerManager().GetTimerRemaining(BeforeGameStartTimerHandle);
	//NEXTTHINGTODO:
	//게임시작까지 남은시간

	CPGameState->SetControlPointGameModeState(ControlPointGameModeState);
	CPGameState->SetTimeCount(LeftTimeBeforeGame);
}

void AControlPointGameMode::BeforePointActivateLeftTimeCheck()
{
	if (!CPGameState)
	{
		CPGameState=Cast<AControlPointGameState>(GetWorld()->GetGameState());
	}
	float LeftTimeBeforeActivateLeftTimeCheck = GetWorldTimerManager().GetTimerRemaining(WaitToTurnOnNextPointTimerHandle);

	ControlPointGameModeState = EControlPointGameModeState::WaitToTurnOnNextPoint;
	CPGameState->SetControlPointGameModeState(ControlPointGameModeState);
	CPGameState->SetTimeCount(LeftTimeBeforeActivateLeftTimeCheck);
}

void AControlPointGameMode::OnGameEnd(uint8 TeamNum)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHPPlayerController* HPPC= Cast<AHPPlayerController>(It->Get());
		if (HPPC)
		{
			HPPC->Client_ShowResultWidget(TeamNum);
		}
	}
}

void AControlPointGameMode::OnControlPointCompleted(EControlPointType CompletedControlPoint, int32 TeamID)
{
	switch (TeamID)
	{
	case 0:
		CurrentTeam1Point++;
		break;
	case 1:
		CurrentTeam2Point++;
		break;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Team1: %d vs Team2: %d"), CurrentTeam1Point, CurrentTeam2Point);
	if (CurrentTeam1Point==VictoryPoint)
	{
		UE_LOG(LogTemp, Warning,TEXT("Team 1 Win"));
		ControlPointGameModeState=EControlPointGameModeState::GameComplete;
		ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(false);
		OnGameEnd(0);
		
	}
	else if (CurrentTeam2Point==VictoryPoint)
	{
		UE_LOG(LogTemp, Warning,TEXT("Team 2 Win"));
		ControlPointGameModeState=EControlPointGameModeState::GameComplete;
		ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(false);
		OnGameEnd(1);
	}

	
	CurrentTargetPointIdx = static_cast<int32>(CompletedControlPoint);
	
	ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(false);
	CurrentTargetPointIdx++;
	CPGameState->OnTeamCompletePoint(CurrentTeam1Point, CurrentTeam2Point, CurrentTargetPointIdx);

	if (ControlPointGameModeState==EControlPointGameModeState::GameComplete)
	{
		return;
	}
	
	TurnOnWaitToTurnOnNextPointTimerHandle(ActivateNextPointTime);
}

void AControlPointGameMode::TurnOnWaitToTurnOnNextPointTimerHandle(float TimerTime)
{
	GetWorldTimerManager().ClearTimer(WaitToTurnOnNextPointTimerHandle);
	GetWorldTimerManager().ClearTimer(SendTimeLeftTimerHandle);

	GetWorldTimerManager().SetTimer(
		WaitToTurnOnNextPointTimerHandle,
		this,
		&AControlPointGameMode::AfterWaitingTimeToActivateNextPoint,
		TimerTime,
		false);
	
	GetWorldTimerManager().SetTimer(
	SendTimeLeftTimerHandle,
	this,
	&AControlPointGameMode::BeforePointActivateLeftTimeCheck,
	0.25f,
	true
	);
}

void AControlPointGameMode::AfterWaitingTimeToActivateNextPoint()
{
	GetWorldTimerManager().ClearTimer(SendTimeLeftTimerHandle);

	ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(true);
	ControlPointGameModeState = EControlPointGameModeState::PointActive;

	CPGameState->SetControlPointGameModeState(ControlPointGameModeState);
}

FGenericTeamId AControlPointGameMode::GetTeamIDForPlayer(const APlayerController* PlayerController) const
{
	static int PlayerCount = 0;
	++PlayerCount;
	return FGenericTeamId(PlayerCount % 2);
}

AActor* AControlPointGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);
	
	if (!StartSpotTag)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	
	for (TActorIterator<ARespawnPlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag &&
			It->GetPlayerStartControlPointNum() == CurrentTargetPointIdx &&
			!It->IsOccupied())
		{
			It->SetOccupied(true);
			return *It;
		}
	}

	for (ARespawnPlayerStart* PlayerStart:RespawnPlayerStarts)
	{
		if (PlayerStart->PlayerStartTag == *StartSpotTag &&
			PlayerStart->GetPlayerStartControlPointNum() == CurrentTargetPointIdx &&
			!PlayerStart->IsOccupied())
		{
			PlayerStart->SetOccupied(true);
			return Cast<AActor>(PlayerStart);
		}
	}
	return nullptr;
}

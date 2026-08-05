// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ControlPointGameMode.h"

#include "EngineUtils.h"
#include "Actors/ControlPointSpawnTargetPoint.h"
#include "Actors/HPControlPoint.h"
#include "Controller/HPPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameMode/ControlPointGameState.h"
#include "Kismet/GameplayStatics.h"

APlayerController* AControlPointGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
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


	ControlPointGameModeState = EControlPointGameModeState::BeforeGameStart;
	CPGameState->SetControlPointGameModeState(ControlPointGameModeState);
	GetWorldTimerManager().SetTimer(
		BeforeGameStartTimerHandle,
		this,
		&AControlPointGameMode::OnGameStart,
		10.f,
		false);

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

void AControlPointGameMode::OnGameStart()
{
	GetWorldTimerManager().ClearTimer(SendTimeLeftTimerHandle);
	BindControllerToControlPoints();

	ControlPointGameModeState = EControlPointGameModeState::WaitToTurnOnNextPoint;
	ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(true);
	CPGameState->SetControlPointGameModeState(ControlPointGameModeState);
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

void AControlPointGameMode::OnControlPointCompleted(EControlPointType CompletedControlPoint, int32 TeamID)
{
	//NEXTTHINGTODO:
	//GameState에 값 넣기

	if (ControlPointGameModeState==EControlPointGameModeState::GameComplete)
	{
		return;
	}
	switch (TeamID)
	{
	case 0:
		CurrentTeam1Point++;
		break;
	case 1:
		CurrentTeam2Point++;
		break;
	}

	CPGameState->OnTeamCompletePoint(CurrentTeam1Point, CurrentTeam2Point);
	
	UE_LOG(LogTemp, Warning, TEXT("Team1: %d vs Team2: %d"), CurrentTeam1Point, CurrentTeam2Point);
	if (CurrentTeam1Point==VictoryPoint)
	{
		UE_LOG(LogTemp, Warning,TEXT("Team 1 Win"));
		ControlPointGameModeState=EControlPointGameModeState::GameComplete;
		ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(false);
		return;
	}
	else if (CurrentTeam2Point==VictoryPoint)
	{
		UE_LOG(LogTemp, Warning,TEXT("Team 2 Win"));
		ControlPointGameModeState=EControlPointGameModeState::GameComplete;
		ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(false);
		return;
	}
	CurrentTargetPointIdx = static_cast<int32>(CompletedControlPoint);
	
	ControlPoints[CurrentTargetPointIdx]->ActivateControlPoint(false);
	CurrentTargetPointIdx++;
	TurnOnWaitToTurnOnNextPointTimerHandle(ActivateNextPointTime);
}

void AControlPointGameMode::TurnOnWaitToTurnOnNextPointTimerHandle(float TimerTime)
{
	GetWorldTimerManager().ClearTimer(WaitToTurnOnNextPointTimerHandle);
	GetWorldTimerManager().ClearTimer(SendTimeLeftTimerHandle);

	GetWorldTimerManager().SetTimer(
	SendTimeLeftTimerHandle,
	this,
	&AControlPointGameMode::BeforePointActivateLeftTimeCheck,
	0.25f,
	true
	);
	
	GetWorldTimerManager().SetTimer(
		WaitToTurnOnNextPointTimerHandle,
		this,
		&AControlPointGameMode::AfterWaitingTimeToActivateNextPoint,
		TimerTime,
		false);

	
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
	
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}
	return nullptr;
}

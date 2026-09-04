// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/HPControlPoint.h"

#include "GenericTeamAgentInterface.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Controller/HPPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/ControlPointWidget.h"


AHPControlPoint::AHPControlPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BoxComponent"));
	SetRootComponent(SphereComponent);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AHPControlPoint::OnSphereBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AHPControlPoint::OnSphereEndOverlap);
	SphereComponent->SetSphereRadius(SphereRadius);

	OnControlPointCaptured = FOnControlPointCaptured(false, false);

	ControlPointWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ControlPointWidgetComponent"));
	ControlPointWidgetComponent->SetupAttachment(GetRootComponent());

	ControlPointWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
}

void AHPControlPoint::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("ControlPoint OVERLAPPED"));
	if (IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(OtherActor))
	{
		if (TeamAgentInterface->GetGenericTeamId().GetId()==0 && !OverlappedTeamOne.Contains(OtherActor))
		{
			OverlappedTeamOne.Add(OtherActor);
			
		}
		else if (TeamAgentInterface->GetGenericTeamId().GetId()==1 && !OverlappedTeamTwo.Contains(OtherActor))
		{
			OverlappedTeamTwo.Add(OtherActor);	
		}
	}
}


void AHPControlPoint::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(OtherActor))
	{
		if (TeamAgentInterface->GetGenericTeamId().GetId()==0 && OverlappedTeamOne.Contains(OtherActor))
		{
			OverlappedTeamOne.Remove(OtherActor);
		}
		else if (TeamAgentInterface->GetGenericTeamId().GetId()==1 && OverlappedTeamTwo.Contains(OtherActor))
		{
			OverlappedTeamTwo.Remove(OtherActor);	
		}
	}
}

void AHPControlPoint::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AHPControlPoint, ControlPointData, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AHPControlPoint, OnControlPointCaptured, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(AHPControlPoint, IsActivating, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AHPControlPoint, ControlPointType, COND_None, REPNOTIFY_Always);
	
}

void AHPControlPoint::OnRep_ControlPointUpdate()
{
	ControlPointUpdateDelegate.ExecuteIfBound(ControlPointData);
}

void AHPControlPoint::OnRep_ControlPointCaptured()
{
	ControlPointCapturedDelegate.ExecuteIfBound(OnControlPointCaptured.TeamOneCaptured, OnControlPointCaptured.TeamTwoCaptured);
}

void AHPControlPoint::BindPlayerControllerToControlPoint(AHPPlayerController* PlayerController)
{
	ControlPointUpdateDelegate.BindDynamic(PlayerController, &AHPPlayerController::UpdateControlPointState);
	ControlPointCapturedDelegate.BindDynamic(PlayerController, &AHPPlayerController::UpdateCapturedTeamState);
}

void AHPControlPoint::UnbindPlayerControllerToControlPoint()
{
	ControlPointUpdateDelegate.Unbind();
	ControlPointCapturedDelegate.Unbind();
	ControlPointCompletedDelegate.Unbind();
}


void AHPControlPoint::BeginPlay()
{
	Super::BeginPlay();
	ControlPointWidgetComponent->SetVisibility(false);
}

void AHPControlPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsActivating)
	{
		return;
	}
	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FVector CameraLocation = FVector::ZeroVector;
		FRotator CameraRotation = FRotator::ZeroRotator;
				
		PC->GetPlayerViewPoint(CameraLocation,CameraRotation);

		if (ControlPointWidget && PC->GetPawn())
		{
			//FVector Direction = CameraLocation-ControlPointWidgetComponent->GetComponentLocation();
			//ControlPointWidgetComponent->SetWorldRotation(Direction.Rotation());
			float Dist = FVector::Dist2D(GetActorLocation(),PC->GetPawn()->GetActorLocation());
			
			Dist-=SphereRadius;
			ControlPointWidget->UpdateDistance(FMath::Max(0,Dist)/100);
		}
	}
	
	if (!HasAuthority())
	{
		return;
	}

	//DrawDebugSphere(GetWorld(), GetActorLocation(), SphereRadius, 12, FColor::Green);

	if (OverlappedTeamOne.Num() >= 1 && OverlappedTeamTwo.Num() >= 1) //한타 중
	{
		UE_LOG(LogTemp, Warning, TEXT("Fighting"));
		CurrentState = EControlPointState::FightingAtPoint;
	}
	
	else if (OverlappedTeamOne.Num() >= 1 && OverlappedTeamTwo.Num() == 0 && CurrentCapturedTeam != Team1Captured) //팀1 탈환중(점령 전)
	{
		UE_LOG(LogTemp, Warning, TEXT("Team1 Capturing"));
		if (/*CurrentState == EControlPointState::BeforeCapturing ||*/ //아무도 거점 점령하지 않았을 때
			CurrentState != EControlPointState::FightingAtPoint )	   
			CurrentState = EControlPointState::Team1Capturing;
		
		if (CurrentTeam2FightingGauge >0.f) //남은 팀2게이지 다 버릴 때까지 
		{
			CurrentTeam2FightingGauge = FMath::Clamp(CurrentTeam2FightingGauge-DeltaTime * FightingStateFillingSpeed, 0, 100);
		}
		else//팀2 게이지 다 버리고 팀1 게이지 채우기
		{
			CurrentTeam1FightingGauge = FMath::Clamp(CurrentTeam1FightingGauge+DeltaTime * FightingStateFillingSpeed, 0, 100);
		}
		
		if (CurrentTeam1FightingGauge >= 100.f)
		{
			CurrentCapturedTeam = EWhatTeamCaptured::Team1Captured;
			CurrentState = EControlPointState::PeaceAtPoint;
			OnControlPointCaptured = FOnControlPointCaptured(true, false);
			CurrentTeam1FightingGauge = 0.f;
			CurrentTeam2FightingGauge = 0.f;
		}
		
	}

	else if (OverlappedTeamOne.Num() == 0 && OverlappedTeamTwo.Num() >= 1 && CurrentCapturedTeam != Team2Captured) //팀2 점령중
	{
		UE_LOG(LogTemp, Warning, TEXT("Team2 Capturing"));
		if (/*CurrentState == EControlPointState::BeforeCapturing ||*/
			CurrentState != EControlPointState::FightingAtPoint)
			CurrentState = EControlPointState::Team2Capturing;

		if (CurrentTeam1FightingGauge >0.f) //남은 팀1게이지 다 버릴 때까지 
		{
			CurrentTeam1FightingGauge = FMath::Clamp(CurrentTeam1FightingGauge-DeltaTime * FightingStateFillingSpeed, 0, 100);
		}
		else //팀1 게이지 다 버리고 팀2 게이지 채우기
		{
			CurrentTeam2FightingGauge = FMath::Clamp(CurrentTeam2FightingGauge+DeltaTime * FightingStateFillingSpeed, 0, 100);
		}

		if (CurrentTeam2FightingGauge >= 100.f)
		{
			CurrentCapturedTeam = EWhatTeamCaptured::Team2Captured;
			CurrentState = EControlPointState::PeaceAtPoint;
			//캡처 끝나면 0으로 초기화
			CurrentTeam1FightingGauge = 0.f;
			CurrentTeam2FightingGauge = 0.f;

			OnControlPointCaptured = FOnControlPointCaptured(false, true);
		}
	}

	else	//아무도 없을 때
	{
		UE_LOG(LogTemp, Warning, TEXT("Peace"));
		CurrentTeam1FightingGauge = FMath::Clamp(CurrentTeam1FightingGauge-DeltaTime* FightingStateFillingSpeed,0,100.f);
		CurrentTeam2FightingGauge = FMath::Clamp(CurrentTeam2FightingGauge-DeltaTime* FightingStateFillingSpeed,0,100.f);

		if (CurrentTeam1CaptureGauge == 0.f && CurrentTeam2CaptureGauge == 0.f) //아무 팀도 거점 못 먹었으면
		{
			CurrentState = EControlPointState::BeforeCapturing;
			CurrentCapturedTeam= EWhatTeamCaptured::None;
		}
		else
		{
			CurrentState = EControlPointState::PeaceAtPoint;
		}
	}
	
	if (CurrentCapturedTeam == EWhatTeamCaptured::Team1Captured) //팀1이 거점 먹은 경우
	{
		CurrentTeam1CaptureGauge+=DeltaTime * ControlledStateFillingSpeed;
		
		if (CurrentTeam1CaptureGauge>=99.f && CurrentState == EControlPointState::FightingAtPoint)
		{
			CurrentTeam1CaptureGauge = 99.f;//싸우고 있을 경우 99퍼센트에서 더 진행되지 않도록
		}
		if (CurrentTeam1CaptureGauge>=100.f)
		{
			//NEXTTHINGTODO: 팀1 승리
			CurrentTeam1CaptureGauge= 0.f;
			CurrentTeam1FightingGauge = 0.f;
			CurrentTeam2CaptureGauge=0.f;
			CurrentTeam2FightingGauge=0.f;
			OnControlPointCaptured = FOnControlPointCaptured(false, false);
			BroadcastWhatTeamCompleteControlPoint(ControlPointType, 0);
		}
	}
	else if (CurrentCapturedTeam == EWhatTeamCaptured::Team2Captured) //팀2가 거점 먹은 경우
	{
		CurrentTeam2CaptureGauge+=DeltaTime * ControlledStateFillingSpeed;

		if (CurrentTeam2CaptureGauge>=99.f && CurrentState == EControlPointState::FightingAtPoint)
		{
			CurrentTeam2CaptureGauge = 99.f;//싸우고 있을 경우 99퍼센트에서 더 진행되지 않도록
		}
		if (CurrentTeam2CaptureGauge>=100.f)
		{
			//NEXTTHINGTODO: 팀2 승리
			CurrentTeam1CaptureGauge = 0.f;
			CurrentTeam1FightingGauge = 0.f;
			CurrentTeam2CaptureGauge = 0.f;
			CurrentTeam2FightingGauge = 0.f;
			OnControlPointCaptured = FOnControlPointCaptured(false, false);
			BroadcastWhatTeamCompleteControlPoint(ControlPointType, 1);
		}
	}

	FHPControlPointData CurrentControlPointData;

	CurrentControlPointData.TeamOneCount = OverlappedTeamOne.Num();
	CurrentControlPointData.TeamTwoCount = OverlappedTeamTwo.Num();
	CurrentControlPointData.TeamOneCapturingPercent = CurrentTeam1CaptureGauge;
	CurrentControlPointData.TeamTwoCapturingPercent = CurrentTeam2CaptureGauge;
	CurrentControlPointData.TeamOneFightingPercent = CurrentTeam1FightingGauge;
	CurrentControlPointData.TeamTwoFightingPercent = CurrentTeam2FightingGauge;

	ControlPointData = CurrentControlPointData;

	UE_LOG(LogTemp, Warning, TEXT("current Point State: %d"), CurrentState);
}

void AHPControlPoint::ActivateControlPoint(bool TurnOn)
{
	SphereComponent->SetCollisionEnabled(TurnOn?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);
	IsActivating = TurnOn;
	if (!TurnOn)
	{
		UnbindPlayerControllerToControlPoint();
	}
}

void AHPControlPoint::BroadcastWhatTeamCompleteControlPoint(EControlPointType InControlPointType, int32 CompleteTeamID)
{
	ControlPointCompletedDelegate.ExecuteIfBound(InControlPointType, CompleteTeamID);
}



void AHPControlPoint::OnRep_ControlPointActivated()
{
	uint8 ControlPointTypeInt = static_cast<uint8>(ControlPointType);
	ControlPointWidget = Cast<UControlPointWidget>(ControlPointWidgetComponent->GetUserWidgetObject());
	ControlPointWidget -> SetControlPointText(FText::FromString(FString::Chr('A' + ControlPointTypeInt)));
	
	ControlPointWidgetComponent->SetVisibility(IsActivating);

	if (!IsActivating)
	{
		ControlPointUpdateDelegate.Unbind();
		ControlPointCapturedDelegate.Unbind();
	}
	
}


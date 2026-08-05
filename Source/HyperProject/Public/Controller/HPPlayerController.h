// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "HPPlayerController.generated.h"

/**
 * 
 */

class AHPControlPointHUD;
class AHPControlPoint;

struct FHPControlPointData;

DECLARE_DYNAMIC_DELEGATE_FourParams(FOnUpdateControlPointDelegate, float, TeamOneFightingPercent, float,
                                    TeamTwoFightingPercent, float, TeamOneCapturingPercent, float,
                                    TeamTwoCapturingPercent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);



UCLASS()
class HYPERPROJECT_API AHPPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(class APawn* InPawn) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	/* IGenericTeamAgentInterface Begin */
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	
	/* IGenericTeamAgentInterface End */

	UFUNCTION(Client, Reliable)
	void Client_OnReceiveEnemyEliminationInfo(FName PlayerNickname,float ContributionPercent);

	UFUNCTION(Client,Reliable)
	void Client_BindControlPoints(const TArray<AHPControlPoint*>& StageControlPoints);

	UFUNCTION()
	void UpdateControlPointState(const FHPControlPointData& ControlPointData);

	
private:
	UPROPERTY()
	class AHPPlayerCharacter* HPPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Replicated)
	FGenericTeamId TeamID;

	/* ControlPointGameMode Begin */
public:
	void BindControlPointGameModeHUD(AHPControlPointHUD* InCPHUD);
	
	
private:
	FOnUpdateControlPointDelegate OnUpdateControlPointDelegate;

	UPROPERTY()
	AHPControlPointHUD* CPHUD;
	/* ControlPointGameMode End */

	/* Ping Begin */
public:
	FHighPingDelegate HighPingDelegate;

	float GetServerTime();
	virtual void ReceivedPlayer() override;

	float SingleTripTime = 0.f;
private:
	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingAnimationDuration = 5.f;//이 시간 동안 애니메이션 재생

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f; //이 시간 마다 핑 체크

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
protected:
	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	

	/* Ping End */
	

};



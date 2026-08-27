// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "HPControlPoint.generated.h"



class USphereComponent;
class AHPPlayerController;
class UWidgetComponent;
class UControlPointWidget;

USTRUCT(BlueprintType)
struct FOnControlPointCaptured
{
	GENERATED_BODY()
	
	FOnControlPointCaptured():TeamOneCaptured(false), TeamTwoCaptured(false) {}
	FOnControlPointCaptured(bool T1Captured, bool T2Captured):TeamOneCaptured(T1Captured), TeamTwoCaptured(T2Captured){}
	UPROPERTY()
	bool TeamOneCaptured;
	UPROPERTY()
	bool TeamTwoCaptured;
};

enum EControlPointState
{
	Team1Capturing,
	Team2Capturing,
	Team1Captured,
	Team2Captured,
	BeforeCapturing,
	FightingAtPoint
};

UENUM(BlueprintType)
enum class EControlPointType : uint8
{
	APoint UMETA(DisplayName = "Point A"),
	BPoint UMETA(DisplayName = "Point B"),
	CPoint UMETA(DisplayName = "Point C")
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnControlPointUpdateDelegate, const FHPControlPointData&, ControlPointData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnControlPointCompletedDelegate, EControlPointType, ControlPointType, int32, CompleteTeamID);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnControlPointCapturedDelegate, bool, TeamOne, bool, TeamTwo);

UCLASS()
class HYPERPROJECT_API AHPControlPoint : public AActor
{
	GENERATED_BODY()
	
public:
	AHPControlPoint();

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ControlPointUpdate();

	UFUNCTION()
	void OnRep_ControlPointCaptured();
	
	void BindPlayerControllerToControlPoint(AHPPlayerController* PlayerController);

	FORCEINLINE void SetControlPointState(EControlPointType InControlPointType) {ControlPointType = InControlPointType;}
	FORCEINLINE EControlPointType GetControlPointType() const {return ControlPointType;}
	FOnControlPointCompletedDelegate ControlPointCompletedDelegate;
	FOnControlPointCapturedDelegate ControlPointCapturedDelegate;
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void ActivateControlPoint(bool TurnOn);
	/*
	 * Tick주기로 짤 경우:
	 * 
	 */
private:
	UPROPERTY(EditDefaultsOnly)
	float SphereRadius;

	UPROPERTY()
	TSet<AActor*> OverlappedTeamOne;
	
	UPROPERTY()
	TSet<AActor*> OverlappedTeamTwo;

	UPROPERTY(EditDefaultsOnly)
	USphereComponent* SphereComponent;

	UPROPERTY(ReplicatedUsing = OnRep_ControlPointUpdate)
	FHPControlPointData ControlPointData;

	UPROPERTY(ReplicatedUsing = OnRep_ControlPointCaptured)
	FOnControlPointCaptured OnControlPointCaptured;
	
	FOnControlPointUpdateDelegate ControlPointUpdateDelegate;

	UPROPERTY(EditAnywhere)
	float FightingStateFillingSpeed = 0.5f;

	UPROPERTY(EditAnywhere)
	float ControlledStateFillingSpeed = 5.f;
	
	float CurrentFightingGauge = 0.f;
	float CurrentControlledGauge = 0.f;
	
	float CurrentTeam1CaptureGauge = 0.f;
	float CurrentTeam2CaptureGauge = 0.f;

	float CurrentTeam1FightingGauge = 0.f;
	float CurrentTeam2FightingGauge = 0.f;

	void BroadcastWhatTeamCompleteControlPoint(EControlPointType ControlPointType, int32 CompleteTeamID);

	EControlPointState CurrentState;

	UPROPERTY(Replicated)
	EControlPointType ControlPointType;
	
	UPROPERTY(EditDefaultsOnly)
	UWidgetComponent* ControlPointWidgetComponent;

	UPROPERTY()
	UControlPointWidget* ControlPointWidget;

	UPROPERTY(ReplicatedUsing = OnRep_ControlPointActivated)
	bool IsActivating = false;

	UFUNCTION()
	void OnRep_ControlPointActivated();
	
};

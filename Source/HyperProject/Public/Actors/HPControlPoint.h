// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "HPControlPoint.generated.h"



class UBoxComponent;
class AHPPlayerController;

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


UCLASS()
class HYPERPROJECT_API AHPControlPoint : public AActor
{
	GENERATED_BODY()
	
public:
	AHPControlPoint();

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ControlPointUpdate();

	void BindPlayerControllerToControlPoint(AHPPlayerController* PlayerController);

	FORCEINLINE void SetControlPointState(EControlPointType InControlPointType) {ControlPointType = InControlPointType;}
	FORCEINLINE EControlPointType GetControlPointType() const {return ControlPointType;}
	FOnControlPointCompletedDelegate ControlPointCompletedDelegate;

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
	FVector BoxExtent;

	UPROPERTY()
	TSet<AActor*> OverlappedTeamOne;
	
	UPROPERTY()
	TSet<AActor*> OverlappedTeamTwo;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* BoxComponent;

	UPROPERTY(ReplicatedUsing = OnRep_ControlPointUpdate)
	FHPControlPointData ControlPointData;

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

	EControlPointType ControlPointType;


	bool IsActivating = false;
};

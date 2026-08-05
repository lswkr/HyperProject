// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUD/HPHUD.h"
#include "HPControlPointHUD.generated.h"

class UHPControlPointOverlayWidget;
class AControlPointGameState;
class AHPPlayerController;
enum class EControlPointGameModeState: uint8;

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API AHPControlPointHUD : public AHPHUD
{
	GENERATED_BODY()

public:
	virtual void InitOverlay(APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS, UHPCombatComponent* CC) override;

	UFUNCTION()
	void OnUpdateControlPoint(float MyTeamFightingPercent, float EnemyTeamFightingPercent, float MyTeamCapturingPercent, float EnemyTeamTwoCapturingPercent);

	UFUNCTION()
	void OnUpdateTimeRemaining(int32 RemainingTime);

	UFUNCTION()
	void OnUpdateCapturePointCount(int32 TeamOneCount, int32 TeamTwoCount);

	UFUNCTION()
	void OnUpdateControlPointGameModeState(EControlPointGameModeState CurrentControlPointGameModeState);

private:
	UPROPERTY()
	UHPControlPointOverlayWidget* ControlPointOverlayWidget;
	
	UPROPERTY()
	AControlPointGameState* ControlPointGameState;

	UPROPERTY()
	AHPPlayerController* HPPlayerController;

	
};

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
	void OnUpdateControlPoint(float TeamOneFightingPercent, float TeamTwoFightingPercent, float TeamOneCapturingPercent, float TeamTwoCapturingPercent);

	UFUNCTION()
	void OnUpdateTimeRemaining(int32 RemainingTime);

	UFUNCTION()
	void OnUpdateCapturePointCount(int32 TeamOneCount, int32 TeamTwoCount);

	UFUNCTION()
	void OnUpdateControlPointGameModeState(EControlPointGameModeState CurrentControlPointGameModeState);
	
	UFUNCTION()
	void OnControlPointCaptured(bool TeamOne, bool TeamTwo);
private:
	UPROPERTY()
	UHPControlPointOverlayWidget* ControlPointOverlayWidget;
	
	UPROPERTY()
	AControlPointGameState* ControlPointGameState;

	UPROPERTY()
	AHPPlayerController* HPPlayerController;

	
};

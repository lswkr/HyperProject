// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/HPInGameOverlayWidget.h"
#include "HPControlPointOverlayWidget.generated.h"

/**
 * 
 */

enum class EControlPointGameModeState: uint8;

UCLASS()
class HYPERPROJECT_API UHPControlPointOverlayWidget : public UHPInGameOverlayWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void FloatTimeRemaining(int32 RemainingTime);
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTeamFightingProgressBar(float MyTeamFightingPercent, float EnemyTeamFightingPercent);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTeamCapturingProgressBar(float MyTeamCapturingPercent, float EnemyTeamCapturingPercent);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTeamCompletePoint(int32 MyTeamCompleteCount, int32 EnemyTeamCompleteCount);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateControlPointGameModeState(EControlPointGameModeState CurrentControlPointGameModeState);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateControlPointCaptured(bool MyTeamCaptured, bool EnemyTeamCaptured);
};

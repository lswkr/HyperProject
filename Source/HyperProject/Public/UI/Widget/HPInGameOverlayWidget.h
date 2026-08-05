// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/HPUserWidget.h"
#include "HPInGameOverlayWidget.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPInGameOverlayWidget : public UHPUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void FloatKillLog(FName VictimName, float ContributionValue);

};

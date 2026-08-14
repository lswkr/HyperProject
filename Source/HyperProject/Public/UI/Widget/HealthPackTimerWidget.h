// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthPackTimerWidget.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHealthPackTimerWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "HealthPackTimerWidget")
	void OnUpdatePercent(float Percent);
	
};

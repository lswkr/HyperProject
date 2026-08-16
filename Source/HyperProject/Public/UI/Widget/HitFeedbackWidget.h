// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HitFeedbackWidget.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHitFeedbackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "HitFeedback")
	void PlayBodyShotAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "HitFeedback")
	void PlayHeadShotAnimation();
	
};

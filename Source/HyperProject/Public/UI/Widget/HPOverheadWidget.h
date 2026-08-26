// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/HPUserWidget.h"
#include "HPOverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPOverheadWidget : public UHPUserWidget
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetColorDependingOnAttitude(bool IsMyTeam);
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScopeWidget.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UScopeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "ScopeWidget")
	void ZoomIn();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "ScopeWidget")
	void ZoomOut();
};

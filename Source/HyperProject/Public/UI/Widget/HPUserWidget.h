// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HPUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	UObject* WidgetController;
};

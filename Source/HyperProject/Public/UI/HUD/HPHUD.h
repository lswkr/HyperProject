// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HPHUD.generated.h"

/**
 * 
 */
class UAbilitySystemComponent;
class UAttributeSet;
class UHPCombatComponent;
class UOverlayWidgetController;
class UHPUserWidget;
class UHPInGameOverlayWidget;

struct FWidgetControllerParams;

UCLASS()
class HYPERPROJECT_API AHPHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void InitOverlay(APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS, UHPCombatComponent* CC);

	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WidgetControllerParams);

	void GetOverlayWidgetFloatKillLog(FName VictimNickName, float ContributionValue);

protected:
	UPROPERTY(EditDefaultsOnly)
	UHPInGameOverlayWidget* OverlayWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHPUserWidget> OverlayWidgetClass;
	
	UPROPERTY()
	UOverlayWidgetController* OverlayWidgetController;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
};

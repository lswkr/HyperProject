// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HPGA_Fire.h"
#include "HPGA_Fire_Aiming.generated.h"

/**
 * 
 */
class UScopeWidget;

UCLASS()
class HYPERPROJECT_API UHPGA_Fire_Aiming : public UHPGameplayAbility
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void ZoomIn(FGameplayEventData Payload);

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* AimStartMontage;
	
	UFUNCTION()
	void CaptureAimStartTime();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UScopeWidget> ScopeWidgetClass;

	UPROPERTY()
	UScopeWidget* ScopeWidget;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "GCNB_SFX_Local.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UGCNB_SFX_Local : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()
	
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;
	
	UPROPERTY(EditDefaultsOnly)
	USoundBase* SoundEffect;
	
	
};

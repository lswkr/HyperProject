// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "GCNB_HitVFX_Particle.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UGCNB_HitVFX_Particle : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

protected:
	
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* ParticleSystem;
	
};

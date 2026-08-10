// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "UHPGameplayCueNS_BulletTrail_PS.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UUHPGameplayCueNS_BulletTrail_PS : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

private:
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* BeamSystem;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "HPGameplayCueNS_BulletTrail.generated.h"

/**
 * 
 */
class UNiagaraSystem;

UCLASS()
class HYPERPROJECT_API UHPGameplayCueNS_BulletTrail : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

private:
	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* BeamSystem;
	
};

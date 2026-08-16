// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "Weapons/AbilitySpawnableActor.h"
#include "AbilitySpawnableActor_Mine.generated.h"

/**
 * 
 */
class UBoxComponent;
class UGameplayEffect;

UCLASS()
class HYPERPROJECT_API AAbilitySpawnableActor_Mine : public AAbilitySpawnableActor
{
	GENERATED_BODY()

public:
	AAbilitySpawnableActor_Mine();
	virtual void SetAbilitySystem(UAbilitySystemComponent* InASC) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	float ExplosionOuterRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	FGameplayTag TargetTag;

	void OnTagChanged(FGameplayTag GameplayTag, int TagCount);

	void Destroy_Normal();
	void Destroy_WithExplosion();

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	UBoxComponent* BoxComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	FScalableFloat ApplicableValue;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	TSubclassOf<UGameplayEffect> ApplicableEffectClass;
	
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Explosion();

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	bool bPush = true;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	float PushPower= 3000.f;
};

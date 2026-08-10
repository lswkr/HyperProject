// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/AbilitySpawnableActor.h"

#include "SpawnableActor_GravitonSurge.generated.h"

/**
 * 
 */
class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class HYPERPROJECT_API ASpawnableActor_GravitonSurge : public AAbilitySpawnableActor
{
	GENERATED_BODY()

public:
	ASpawnableActor_GravitonSurge();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	USphereComponent* GravitySphere;

	UPROPERTY(EditDefaultsOnly)
	float SurgeRadius;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	TSet<AActor*> OverlappedActors;

	void TryAddTarget(AActor* OtherActor);
	void RemoveTarget(AActor* OtherActor);

	UPROPERTY(EditDefaultsOnly)
	float SurgeSpeed;
	
	UPROPERTY(EditDefaultsOnly)
	float SurgeDuration;

	FTimerHandle SurgeDurationTimerHandle;

	void OnTimerExpired();

	UPROPERTY(EditDefaultsOnly)
	UNiagaraComponent* SurgeNiagaraComponent;
};

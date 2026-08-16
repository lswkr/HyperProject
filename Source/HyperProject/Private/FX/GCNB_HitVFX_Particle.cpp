// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/GCNB_HitVFX_Particle.h"

#include "Kismet/GameplayStatics.h"


bool UGCNB_HitVFX_Particle::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	UGameplayStatics::SpawnEmitterAtLocation(
		Target,
		ParticleSystem,
		Parameters.Location
		);
	return Super::OnExecute_Implementation(Target, Parameters);
}

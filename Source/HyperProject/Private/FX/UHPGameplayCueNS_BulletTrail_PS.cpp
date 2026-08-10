// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/UHPGameplayCueNS_BulletTrail_PS.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

bool UUHPGameplayCueNS_BulletTrail_PS::OnExecute_Implementation(AActor* MyTarget,
                                                                const FGameplayCueParameters& Parameters) const
{

	if (!MyTarget||!BeamSystem)
		return false;

	FVector BeamStart = Parameters.EffectContext.GetHitResult()->Location;
	FVector BeamEnd = Parameters.EffectContext.GetHitResult()->ImpactPoint;

	
	if (GetWorld() && BeamSystem)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			BeamSystem,
			BeamStart,
			FRotator::ZeroRotator,
			true
		);
		if (Beam)
		{
			Beam->SetVectorParameter(FName("Target"), BeamEnd);
		}
		return true;
	}
	return false;
}

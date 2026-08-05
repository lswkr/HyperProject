// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/HPGameplayCueNS_BulletTrail.h"

#include "Weapons/HPWeaponBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

bool UHPGameplayCueNS_BulletTrail::OnExecute_Implementation(AActor* MyTarget,
                                                            const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget||!BeamSystem)
		return false;

	FVector BeamStart = Parameters.EffectContext.GetHitResult()->Location;
	FVector BeamEnd = Parameters.EffectContext.GetHitResult()->ImpactPoint;

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget, BeamSystem, BeamStart);

	if (!NiagaraComponent)
		return false;

	NiagaraComponent->SetVectorParameter(FName("Beam End"),BeamEnd);
		return true;
}

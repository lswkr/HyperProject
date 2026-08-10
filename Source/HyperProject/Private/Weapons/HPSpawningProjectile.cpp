// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPSpawningProjectile.h"

#include "Weapons/AbilitySpawnableActor.h"
#include "Weapons/HPWeaponBase.h"

void AHPSpawningProjectile::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
		return;
	
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Hit.ImpactPoint);
	SpawnTransform.SetRotation(FQuat::Identity);
	SpawnTransform.SetScale3D(FVector::OneVector);
	
	if (Hit.bBlockingHit && GetOwner() && GetInstigator())
	{
		AAbilitySpawnableActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AAbilitySpawnableActor>(
			SpawnedActorClass,
			SpawnTransform,
			GetOwner(),
			GetInstigator(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);
		
		SpawnedActor->SetAbilitySystem(ProjectileParams.SourceASC);
		// if (AHPProjectileBase* SpawnedProjectile = Cast<AHPProjectileBase>(SpawnedActor))
		// {
		// 	
		// 	SpawnedProjectile->SetProjectileEffectParams(ProjectileParams);
		// 	if (SpawnedProjectile->IsMine())
		// 	{
		// 		SpawnedProjectile->BindExplosionCallbackFunction(GetOwner());
		// 	}
		// }
		SpawnedActor->SetGenericTeamId(GetGenericTeamId());
		SpawnedActor->FinishSpawning(SpawnTransform);
	}
}

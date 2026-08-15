// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPSpawningProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/LagCompensationComponent.h"
#include "Controller/HPPlayerController.h"
#include "Weapons/AbilitySpawnableActor.h"
#include "Weapons/HPWeaponBase.h"

void AHPSpawningProjectile::MakeProjectileEffectParams(FProjectileApplyEffectParams& ProjectileApplyEffectParams)
{
	Super::MakeProjectileEffectParams(ProjectileApplyEffectParams);
	ProjectileApplyEffectParams.SpawnableActorClass = SpawnedActorClass;
}

void AHPSpawningProjectile::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
		return;

	if (!bServerSideRewind)
	{
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
		return;
	}
	AHPPlayerCharacter* OwnerCharacter = Cast<AHPPlayerCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AHPPlayerController* OwnerController = Cast<AHPPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			AHPPlayerCharacter* HitCharacter = Cast<AHPPlayerCharacter>(OtherActor);
			if (bServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
			{
				// if (HitCharacter) //캐릭터에 닿았을 경우
				// {
				// 	UAbilitySystemComponent* SourceASC = ProjectileParams.SourceASC;
				// 	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitCharacter);
				//
				// 	FProjectileApplyEffectParams ProjectileApplyEffectParams;
				// 	ProjectileApplyEffectParams.SourceASC=SourceASC;
				// 	ProjectileApplyEffectParams.TargetASC=TargetASC;
				// 	ProjectileApplyEffectParams.DamageEffectClass = DamageEffectClass;
				// 	ProjectileApplyEffectParams.AdditionalEffectClass = AdditionalEffectClass;
				// 	ProjectileApplyEffectParams.Damage = Damage.GetValueAtLevel(1);
				// 	ProjectileApplyEffectParams.AdditionalEffectValue = AdditionalValue.GetValueAtLevel(1);
				// 	ProjectileApplyEffectParams.bAdditionalEffectForTeam = bIsForMyTeam;
				// 	ProjectileApplyEffectParams.GenericTeamId = GetGenericTeamId();
				// 	
				// 	OwnerCharacter->GetLagCompensationComponent()->ProjectileServerApplyValidHit(
				// 		HitCharacter,
				// 		TraceStart,
				// 		InitialVelocity,
				// 		OwnerController->GetServerTime() - OwnerController->SingleTripTime,
				// 		ProjectileApplyEffectParams
				// 	);
				// }
				//else //오브젝트에 닿았을 경우
				//{
				UAbilitySystemComponent* SourceASC = ProjectileParams.SourceASC;
				
				FProjectileApplyEffectParams ProjectileApplyEffectParams;
				ProjectileApplyEffectParams.SourceASC=SourceASC;
				ProjectileApplyEffectParams.SourceCharacter = OwnerCharacter;
				ProjectileApplyEffectParams.SpawnableActorClass = SpawnedActorClass;
				ProjectileApplyEffectParams.OriginLocation = Hit.ImpactPoint;
				ProjectileApplyEffectParams.GenericTeamId = GetGenericTeamId();
				OwnerCharacter->GetLagCompensationComponent()->SpawningProjectileServerApplyValidHit(TraceStart, InitialVelocity, ProjectileApplyEffectParams);
				//}
			}
		}
	}
	Destroy();
}

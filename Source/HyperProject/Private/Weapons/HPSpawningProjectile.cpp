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
	//ProjectileApplyEffectParams.bIsBounded = bNeedToBeBounded;
}

void AHPSpawningProjectile::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bServerSideRewind)
	{
		if (!HasAuthority())
			return;
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation());
		SpawnTransform.SetRotation(Hit.ImpactNormal.ToOrientationQuat());
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
				if (HitCharacter) //캐릭터에 닿았을 경우
				{
					UAbilitySystemComponent* SourceASC = ProjectileParams.SourceASC;
					UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitCharacter);
					
					FProjectileApplyEffectParams ProjectileApplyEffectParams;
					MakeProjectileEffectParams(ProjectileApplyEffectParams);
					ProjectileApplyEffectParams.TargetCharacter = HitCharacter;
					ProjectileApplyEffectParams.TargetASC=TargetASC;
					ProjectileApplyEffectParams.OriginLocation = GetActorLocation();
					ProjectileApplyEffectParams.HitImpactPoint = Hit.ImpactPoint;
					ProjectileApplyEffectParams.HitImpactNormal = Hit.ImpactNormal; 
					OwnerCharacter->GetLagCompensationComponent()->SpawningProjectileServerApplyValidHit_HitCharacter
					(	HitCharacter,
						TraceStart,
						InitialVelocity,
						OwnerController->GetServerTime() - OwnerController->SingleTripTime,
						ProjectileApplyEffectParams
						);
				}
				else //오브젝트에 닿았을 경우
				{
					FProjectileApplyEffectParams ProjectileApplyEffectParams;

					MakeProjectileEffectParams(ProjectileApplyEffectParams);
					ProjectileApplyEffectParams.OriginLocation = GetActorLocation();
					ProjectileApplyEffectParams.HitImpactPoint = Hit.ImpactPoint;
					ProjectileApplyEffectParams.HitImpactNormal = Hit.ImpactNormal; 
					OwnerCharacter->GetLagCompensationComponent()->SpawningProjectileServerApplyValidHit_HitObject(
						TraceStart,
						InitialVelocity,
						ProjectileApplyEffectParams
						);
				}
			}
		}
	}
	Destroy();
}

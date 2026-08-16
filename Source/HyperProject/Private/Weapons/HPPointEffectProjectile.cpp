// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPPointEffectProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/LagCompensationComponent.h"
#include "Controller/HPPlayerController.h"
#include "Kismet/GameplayStatics.h"

void AHPPointEffectProjectile::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bServerSideRewind)
	{
		AHPPlayerCharacter* OwnerCharacter = Cast<AHPPlayerCharacter>(GetOwner());
		if (OwnerCharacter)
		{
			AHPPlayerController* OwnerController = Cast<AHPPlayerController>(OwnerCharacter->Controller);
			if (OwnerController)
			{
			if(LocalParticleEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					this,
					LocalParticleEffect,
					Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation());
			
			}
				
				AHPPlayerCharacter* HitCharacter = Cast<AHPPlayerCharacter>(OtherActor);
				if (bServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
				{
					FProjectileApplyEffectParams ProjectileApplyEffectParams;
					MakeProjectileEffectParams(ProjectileApplyEffectParams);
				
					OwnerCharacter->GetLagCompensationComponent()->ProjectileServerApplyValidHit(
						HitCharacter,
						TraceStart,
						InitialVelocity,
						ProjectileApplyEffectParams,
						OwnerController->GetServerTime() - OwnerController->SingleTripTime
					);
				
				}
			}
		}
	}
	Super::OnBoxComponentHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

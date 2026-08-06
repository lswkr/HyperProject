// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPPointEffectProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/LagCompensationComponent.h"
#include "Controller/HPPlayerController.h"

void AHPPointEffectProjectile::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AHPPlayerCharacter* OwnerCharacter = Cast<AHPPlayerCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AHPPlayerController* OwnerController = Cast<AHPPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			AHPPlayerCharacter* HitCharacter = Cast<AHPPlayerCharacter>(OtherActor);
			if (bServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				UAbilitySystemComponent* SourceASC = ProjectileParams.SourceASC;
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitCharacter);

				FProjectileApplyEffectParams ProjectileApplyEffectParams;
				ProjectileApplyEffectParams.SourceASC=SourceASC;
				ProjectileApplyEffectParams.TargetASC=TargetASC;
				ProjectileApplyEffectParams.DamageEffectClass = DamageEffectClass;
				ProjectileApplyEffectParams.AdditionalEffectClass = AdditionalEffectClass;
				ProjectileApplyEffectParams.Damage = Damage.GetValueAtLevel(1);
				ProjectileApplyEffectParams.AdditionalEffectValue = AdditionalValue.GetValueAtLevel(1);

				OwnerCharacter->GetLagCompensationComponent()->ProjectileServerApplyValidHit(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime,
					ProjectileApplyEffectParams
				);
				
			}
		}
	}
	Super::OnBoxComponentHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPRadialEffectProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "HPGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/Player/HPPlayerCharacter.h"

AHPRadialEffectProjectile::AHPRadialEffectProjectile()
{
	
}


void AHPRadialEffectProjectile::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bIsMine) //지뢰일 경우 팀이 다르면 폭발
	{
		//NEXTTHINGTODO: 폭발할 때 브로드캐스트하는 델리게이트 걸기
		if (IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(OtherActor))
		{
			if (TeamAgentInterface->GetTeamAttitudeTowards(*this) == ETeamAttitude::Hostile)
			{
				Destroyed();
			}
		}
	}
	else
	{
		Destroyed();
	}
}

void AHPRadialEffectProjectile::ExplodeProjectile()
{
	APawn* FiringPawn = GetInstigator();

	if (FiringPawn && HasAuthority())
	{
		FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
		UAbilitySystemComponent* SourceASC = ProjectileParams.SourceASC;
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();

		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);
		IgnoreActors.Add(FiringPawn);
		
		FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(ApplyRadialDamage),  false, this);

		TArray<FOverlapResult> Overlaps;
		DrawDebugSphere(GetWorld(),GetActorLocation(), ExplosionOuterRadius, 16, FColor::Green,false, 5, 0,1 );
		if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
		{
			World->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(ExplosionOuterRadius), SphereParams);
		}

		TMap<AActor*, float> OverlappedActorsDistanceMap;
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* OverlappedActor = Overlap.GetActor();
			
			if (OverlappedActor && OverlappedActor != this)
			{
				if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor))
				{
					float DistFromOrigin = FVector::Dist(GetActorLocation(), OverlappedActor->GetActorLocation());
					
					OverlappedActorsDistanceMap.Add(OverlappedActor,DistFromOrigin - ExplosionInnerRadius);
				}
			}
		}
	
		if (OverlappedActorsDistanceMap.Num() > 0)
		{
			for (TPair<AActor*, float> Pair : OverlappedActorsDistanceMap)
			{
				float RangeRate = Pair.Value/(ExplosionOuterRadius-ExplosionInnerRadius);

				UAbilitySystemComponent* OverlappedCharacterASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pair.Key);

				if (OverlappedCharacterASC)
				{
					float FinalDamage = Damage.GetValueAtLevel(1);
					FinalDamage*= (1-RangeRate); //Damage는 MaxDamage로 해서
					FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass,1,Context);
					UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, FinalDamage);

					//NEXTTHINGTODO: 모두 다 Overlap시킨 뒤 Team인지 아닌지 구별해서 Team인 경우 Additional Effect적용하도록
					
					// if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(HPCharacter))
					// {
					// 	ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult->GetActor());
					// 	if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
					// 	{
					OverlappedCharacterASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
					
					if (AdditionalEffectClass)
					{
						FGameplayEffectSpecHandle AdditionalEffectSpecHandle = SourceASC->MakeOutgoingSpec(AdditionalEffectClass,1,Context);
						OverlappedCharacterASC->ApplyGameplayEffectSpecToSelf(*AdditionalEffectSpecHandle.Data);
					}
		
					// 	}
					// }
					
				}
			}

		}
	}
}

void AHPRadialEffectProjectile::Destroyed()
{
	ExplodeProjectile();
	Super::Destroyed();
}



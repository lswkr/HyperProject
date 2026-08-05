// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/AbilitySpawnableActor_Mine.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "HPGameplayTags.h"
#include "Components/BoxComponent.h"
#include "Engine/OverlapResult.h"
#include "GameplayEffect.h"

AAbilitySpawnableActor_Mine::AAbilitySpawnableActor_Mine()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AAbilitySpawnableActor_Mine::OnBoxBeginOverlap);
}

void AAbilitySpawnableActor_Mine::SetAbilitySystem(UAbilitySystemComponent* InASC)
{
	Super::SetAbilitySystem(InASC);

	if (HasAuthority())
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(
			TargetTag,
			EGameplayTagEventType::NewOrRemoved
			).AddUObject(this, &AAbilitySpawnableActor_Mine::OnTagChanged);
	}
}

void AAbilitySpawnableActor_Mine::OnTagChanged(FGameplayTag GameplayTag, int TagCount)
{
	if (TagCount == 0)
	{
		Destroy_Normal();	
	}
}

void AAbilitySpawnableActor_Mine::Destroy_Normal()
{
	if (!HasAuthority())
		return;
	UE_LOG(LogTemp, Warning, TEXT("Bomb Eliminated"));
	Destroy();
}

void AAbilitySpawnableActor_Mine::Destroy_WithExplosion()
{
	if (!HasAuthority())
		return;
	
	
	Explosion();
	Destroy();
}

void AAbilitySpawnableActor_Mine::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		//NEXTTHINGTODO: 같은 팀이면 지나가기, 다른 팀이면 폭발
		//일단은 닿으면 폭발하도록
		
		//Destroy_WithExplosion();
	}
}

void AAbilitySpawnableActor_Mine::Explosion()
{
	APawn* FiringPawn = GetInstigator();

	if (FiringPawn && HasAuthority())
	{
		UE_LOG(LogTemp, Warning,TEXT("Mine Explosion"));
		FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
		UAbilitySystemComponent* SourceASC = AbilitySystemComponent;
		if (!SourceASC)
			return;
		
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
					
					OverlappedActorsDistanceMap.Add(OverlappedActor,DistFromOrigin);
				}
			}
		}
		checkf(ApplicableEffectClass, TEXT("Applicable effect class didn't set in ASAMine."))
		if (OverlappedActorsDistanceMap.Num() > 0)
		{
			for (TPair<AActor*, float> Pair : OverlappedActorsDistanceMap)
			{
				float RangeRate = Pair.Value/(ExplosionOuterRadius);

				UAbilitySystemComponent* OverlappedCharacterASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pair.Key);

				if (OverlappedCharacterASC)
				{
					float FinalDamage = ApplicableValue.GetValueAtLevel(1);
					FinalDamage*= (1-RangeRate); //Damage는 MaxDamage로 해서
					FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(ApplicableEffectClass,1,Context);
					UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, FinalDamage);

					//NEXTTHINGTODO: 모두 다 Overlap시킨 뒤 Team인지 아닌지 구별해서 Team인 경우 Additional Effect적용하도록
					
					// if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(HPCharacter))
					// {
					// 	ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult->GetActor());
					// 	if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
					// 	{
					OverlappedCharacterASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
		
					// 	}
					// }
					
				}
			}

		}
	}
}

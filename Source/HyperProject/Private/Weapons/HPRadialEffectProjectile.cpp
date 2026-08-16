// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPRadialEffectProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "HPGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/LagCompensationComponent.h"
#include "Controller/HPPlayerController.h"
#include "Kismet/GameplayStatics.h"

AHPRadialEffectProjectile::AHPRadialEffectProjectile()
{
	
}

void AHPRadialEffectProjectile::MakeProjectileEffectParams(FProjectileApplyEffectParams& ProjectileApplyEffectParams)
{
	Super::MakeProjectileEffectParams(ProjectileApplyEffectParams);
	ProjectileApplyEffectParams.OuterRadius=ExplosionOuterRadius;
	ProjectileApplyEffectParams.InnerRadius=ExplosionInnerRadius;
	ProjectileApplyEffectParams.bCanPush = bCanPush;
	ProjectileApplyEffectParams.PushPower = PushPower;
	ProjectileApplyEffectParams.bDistanceFalloff = bDistanceFalloff;
	
	//Origin은 피격 시점에 채우기
}


void AHPRadialEffectProjectile::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();

	if (FiringPawn)
	{
		FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
		UAbilitySystemComponent* SourceASC = ProjectileParams.SourceASC;
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();

		if (Hit.bBlockingHit)
		{
			if(LocalParticleEffect)
			{
		UGameplayStatics::SpawnEmitterAtLocation(
					this,
					LocalParticleEffect,
					Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation());
			}
				
		}
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
		TSet<AHPPlayerCharacter*> HPCharacterSet; //LagCompensation위해 HPPlayerCharacter로 캐스트 필요
		
		for (const FOverlapResult& Overlap : Overlaps)
		{
			// AActor* OverlappedActor = Overlap.GetActor();
			//
			// if (OverlappedActor && OverlappedActor != this)
			// {
			// 	if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor))
			// 	{
			// 		//float DistFromOrigin = FVector::Dist(GetActorLocation(), OverlappedActor->GetActorLocation());
			// 		
			// 		//OverlappedActorsDistanceMap.Add(OverlappedActor,DistFromOrigin - ExplosionInnerRadius);
			// 	}
			// }
			if (AHPPlayerCharacter* HPPlayerCharacter = Cast<AHPPlayerCharacter>(Overlap.GetActor()))
			{
				HPCharacterSet.Add(HPPlayerCharacter);
			}
		}
	
		if (HPCharacterSet.Num() > 0)
		{

			// for (TPair<AActor*, float> Pair : OverlappedActorsDistanceMap)
			// {
			// 	//float RangeRate = Pair.Value/(ExplosionOuterRadius-ExplosionInnerRadius);
			//
			// 	UAbilitySystemComponent* OverlappedCharacterASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pair.Key);
			//
			// 	if (OverlappedCharacterASC)
			// 	{
			// 		// float FinalDamage = Damage.GetValueAtLevel(1);
			// 		// FinalDamage*= (1-RangeRate); //Damage는 MaxDamage로 해서
			// 		// FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass,1,Context);
			// 		// UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, FinalDamage);
			//
			// 		//NEXTTHINGTODO: 모두 다 Overlap시킨 뒤 Team인지 아닌지 구별해서 Team인 경우 Additional Effect적용하도록
			//
			// 		
			// 		// if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(HPCharacter))
			// 		// {
			// 		// 	ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult->GetActor());
			// 		// 	if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
			// 		// 	{
			// 		//OverlappedCharacterASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
			// 		
			// 		// if (AdditionalEffectClass)
			// 		// {
			// 		// 	FGameplayEffectSpecHandle AdditionalEffectSpecHandle = SourceASC->MakeOutgoingSpec(AdditionalEffectClass,1,Context);
			// 		// 	OverlappedCharacterASC->ApplyGameplayEffectSpecToSelf(*AdditionalEffectSpecHandle.Data);
			// 		// }
			// 		// if (bPushEnemy)
			// 		// {
			// 		// 	FVector HitCharacterLocation = Pair.Key->GetActorLocation();
			// 		// 	FVector Origin = GetActorLocation();
			// 		//
			// 		// 	FVector PushDirection = (HitCharacterLocation - Origin).GetSafeNormal();
			// 		//
			// 		// 	if (ACharacter* HitCharacter = Cast<ACharacter>(Pair.Key))
			// 		// 	{
			// 		// 		HitCharacter->LaunchCharacter(PushDirection * PushPower, true,true);	
			// 		// 	}
			// 		// 	
			// 		// }
			// 		// 	}
			// 		// }
			// 		
			// 	}
			// }
			if (bServerSideRewind)
			{
				AHPPlayerCharacter* OwnerCharacter = Cast<AHPPlayerCharacter>(GetOwner());
				AHPPlayerController* OwnerController = Cast<AHPPlayerController>(OwnerCharacter->Controller);
			
				if (OwnerCharacter && OwnerController)
				{
					AHPPlayerCharacter* HitCharacter = Cast<AHPPlayerCharacter>(OtherActor); //처음 맞은 캐릭터

					//캐릭터가 맞은 경우
					if (HitCharacter && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
					{
				
						FProjectileApplyEffectParams ProjectileApplyEffectParams;
						MakeProjectileEffectParams(ProjectileApplyEffectParams);

						ProjectileApplyEffectParams.OriginLocation = Hit.ImpactPoint;
						//ProjectileApplyEffectParams.bAdditionalEffectForTeam = bIsForMyTeam;
						
						OwnerCharacter->GetLagCompensationComponent()->ExplosionServerApplyValidHit_HitCharacter(
							HitCharacter,
							TraceStart,
							InitialVelocity,
							OwnerController->GetServerTime() - OwnerController->SingleTripTime,
							ProjectileApplyEffectParams,
							HPCharacterSet.Array()
						);
			
					}

					else
					{
						FProjectileApplyEffectParams ProjectileApplyEffectParams;
						MakeProjectileEffectParams(ProjectileApplyEffectParams);
						ProjectileApplyEffectParams.OriginLocation = Hit.ImpactPoint;
						OwnerCharacter->GetLagCompensationComponent()->ExplosionServerApplyValidHit_HitObject(
							TraceStart,
							InitialVelocity,
							OwnerController->GetServerTime() - OwnerController->SingleTripTime,
							ProjectileApplyEffectParams,
							HPCharacterSet.Array()
						);
					}
				}
		
			}
		}
		
	}
	Destroy();
		
}
//NextThingTODO: SSR아닐 때 판정 구현하기
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
					float FinalDamage = EnemyEffectValue.GetValueAtLevel(1);
					FinalDamage*= (1-RangeRate); //Damage는 MaxDamage로 해서
					FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(EnemyEffectClass,1,Context);
					UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, FinalDamage);

					//NEXTTHINGTODO: 모두 다 Overlap시킨 뒤 Team인지 아닌지 구별해서 Team인 경우 Additional Effect적용하도록
					
					// if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(HPCharacter))
					// {
					// 	ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult->GetActor());
					// 	if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
					// 	{
					OverlappedCharacterASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
					
					if (TeamEffectClass)
					{
						FGameplayEffectSpecHandle AdditionalEffectSpecHandle = SourceASC->MakeOutgoingSpec(TeamEffectClass,1,Context);
						OverlappedCharacterASC->ApplyGameplayEffectSpecToSelf(*AdditionalEffectSpecHandle.Data);
					}
					if (bCanPush)
					{
						FVector HitCharacterLocation = Pair.Key->GetActorLocation();
						FVector Origin = GetActorLocation();

						FVector PushDirection = (HitCharacterLocation - Origin).GetSafeNormal();

						if (ACharacter* HitCharacter = Cast<ACharacter>(Pair.Key))
						{
							HitCharacter->LaunchCharacter(PushDirection * PushPower, true,true);	
						}
						
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
	//ExplodeProjectile();
	Super::Destroyed();
}



// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/AbilitySpawnableActor_Mine.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "HPGameplayTags.h"
#include "Components/BoxComponent.h"
#include "Engine/OverlapResult.h"
#include "GameplayEffect.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

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
 	if (TagCount == 1)
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
	
		IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetInstigator());

		if (OwnerTeamInterface)
		{
			BoxComponent->SetSimulatePhysics(true);
			ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*OtherActor);
			if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
			{
				Destroy_WithExplosion();
			}	
		}
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
		Multicast_Explosion();
		TArray<FOverlapResult> Overlaps;
		//DebugSphere(GetWorld(),GetActorLocation(), ExplosionOuterRadius, 16, FColor::Green,false, 5, 0,1 );
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

				AHPPlayerCharacter* OverlappedCharacter = Cast<AHPPlayerCharacter>(Pair.Key);
				//UAbilitySystemComponent* OverlappedCharacterASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pair.Key);

				if (OverlappedCharacter)
				{
					float FinalDamage = ApplicableValue.GetValueAtLevel(1);
					FinalDamage*= (1-RangeRate); //Damage는 MaxDamage로 해서
					FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(ApplicableEffectClass,1,Context);
					UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, FinalDamage);

					//NEXTTHINGTODO: 모두 다 Overlap시킨 뒤 Team인지 아닌지 구별해서 Team인 경우 Additional Effect적용하도록

					if (bPush)
					{
						FRotator NewRotation = (OverlappedCharacter->GetActorLocation() - GetActorLocation()).Rotation();
						OverlappedCharacter->LaunchCharacter(NewRotation.Vector()* PushPower, true, true);
					}
					 if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(FiringPawn))
					 {
					 	ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*Pair.Key);
					 	if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
					 	{
							OverlappedCharacter->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
						}
					}
					
				}
			}

		}
	}
}

void AAbilitySpawnableActor_Mine::Multicast_Explosion_Implementation()
{
	if (!ExplosionParticle)
		return;
	UGameplayStatics::SpawnEmitterAtLocation(
			this,
			ExplosionParticle,
			GetActorLocation(),
			FRotator::ZeroRotator,
			true
			);

	UGameplayStatics::PlaySoundAtLocation(
        this,
        ExplosionSound,
        GetActorLocation()
    );
}

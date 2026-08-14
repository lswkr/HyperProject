// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/HPHealthPack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


// Sets default values
AHPHealthPack::AHPHealthPack()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHPHealthPack::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHPHealthPack::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

	if (ASC)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GameplayEffectClass, 1, ContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		Super::OnBoxBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
}


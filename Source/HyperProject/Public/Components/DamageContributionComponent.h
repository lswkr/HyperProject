// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageContributionComponent.generated.h"

class UHPAbilitySystemComponent;
class APlayerState;



USTRUCT(Blueprintable)
struct FDamageContributionData
{
	GENERATED_BODY()

	FDamageContributionData()
	{
		ContributorPS = nullptr; 
		AppliedDamage = 0.f;
		DamagedTime = 0.f;
	}
	
	FDamageContributionData(TWeakObjectPtr<APlayerState> PS, float Damage, float DT): ContributorPS(PS), AppliedDamage(Damage), DamagedTime(DT){}

	//닉네임 등의 처리를 위해 PlayerState를 활용
	UPROPERTY()
	TWeakObjectPtr<APlayerState> ContributorPS = nullptr;

	float AppliedDamage = 0.f;
	
	float DamagedTime = 0.f;
};

using FNode = TDoubleLinkedList<FDamageContributionData>::TDoubleLinkedListNode;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HYPERPROJECT_API UDamageContributionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageContributionComponent();

protected:
	virtual void BeginPlay() override;

public:
	void AddDamageContributionData(const FDamageContributionData& InData);
	void ResetDamageContributionData();
	void BindAbilitySystemComponent(UHPAbilitySystemComponent* ASC);
	void ProcessAllDatas();

	void SpreadKillLogs(float MaxHealth);
	
private:
	TDoubleLinkedList<FDamageContributionData> DamageContributions;

	const int32 CONTRIBUTION_MAX_SIZE = 500;
};

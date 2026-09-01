// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DamageContributionComponent.h"

#include "GameplayEffectExtension.h"
#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "Controller/HPPlayerController.h"
#include "GameFramework/PlayerState.h"


struct FOnAttributeChangeData;

UDamageContributionComponent::UDamageContributionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UDamageContributionComponent::BeginPlay()
{
	Super::BeginPlay();

	

}

void UDamageContributionComponent::AddDamageContributionData(const FDamageContributionData& InData)
{
	if (DamageContributions.IsEmpty())//아무 원소가 없다면
	{
		//힐이 들어올 경우 그냥 return;
		if (InData.AppliedDamage < 0)
		{
			return;
		}
		else
		{
			DamageContributions.AddTail(InData);
		}
	}
	else
	{
		if (InData.AppliedDamage < 0) //힐 들어올 경우 처음부터 시작해 기록 상쇄
		{
			float RemainingHeal = InData.AppliedDamage*(-1);

			while (RemainingHeal > KINDA_SMALL_NUMBER && !DamageContributions.IsEmpty())
			{
				FNode* IterNode = DamageContributions.GetHead();
				
				if (IterNode->GetValue().AppliedDamage>RemainingHeal)
				{
					IterNode->GetValue().AppliedDamage-=RemainingHeal;
					return;
				}
				else
				{
					RemainingHeal-=IterNode->GetValue().AppliedDamage;
					DamageContributions.RemoveNode(IterNode);
				}
			}
		}

		else
		{
			if (DamageContributions.Num()==CONTRIBUTION_MAX_SIZE) //최적화를 위해 사이즈가 차면 삭제
			{
				DamageContributions.RemoveNode(DamageContributions.GetHead());
			}
			FNode* TailNode = DamageContributions.GetTail();

			if (TailNode->GetValue().ContributorPS->GetPawn() == InData.ContributorPS->GetPawn())
			{
				TailNode->GetValue().AppliedDamage += InData.AppliedDamage;

				/*
				 * PS: 같은 캐릭터면 연속된 데미지이기에 시간이 어느 정도 지났어도
				 * 최근 데미지 시간으로 초기화하여도 이후 기여도 계산 시 반영에 문제가 없다고 생각 
				 */
				TailNode->GetValue().DamagedTime = InData.DamagedTime;
			}
			else
			{
				DamageContributions.AddTail(InData);
			}
		}
	}
}

void UDamageContributionComponent::ResetDamageContributionData()
{
	DamageContributions.Empty();
}

void UDamageContributionComponent::BindAbilitySystemComponent(UHPAbilitySystemComponent* ASC)
{
	ASC->GetGameplayAttributeValueChangeDelegate(UHPAttributeSet::GetIncomingDamageAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			if (Data.NewValue==0.f)
				return;
			FDamageContributionData DCData;
			DCData.AppliedDamage = Data.NewValue;
			DCData.DamagedTime = GetWorld()->GetTimeSeconds();
			const FGameplayEffectContextHandle& Context = Data.GEModData->EffectSpec.GetContext();
			
			
			UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();

			if (!SourceASC)
			{
				return;
			}

			APawn* SourcePawn = Cast<APawn>(SourceASC->GetAvatarActor());
			UE_LOG(LogTemp, Warning, TEXT("Source Character: %s"), *SourcePawn->GetActorNameOrLabel());
			APlayerState* SourcePlayerState = SourcePawn ? SourcePawn->GetPlayerState() : nullptr;
			DCData.ContributorPS = SourcePlayerState;

			AddDamageContributionData(DCData);
		}
	);

	ASC->GetGameplayAttributeValueChangeDelegate(UHPAttributeSet::GetIncomingHealAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			if (Data.NewValue==0.f)
				return;
			
			FDamageContributionData DCData;
			DCData.AppliedDamage = Data.NewValue* (-1);
			DCData.DamagedTime = GetWorld()->GetTimeSeconds();
			const FGameplayEffectContextHandle& Context = Data.GEModData->EffectSpec.GetContext();
			
			UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();

			if (!SourceASC)
			{
				return;
			}

			APawn* SourcePawn = Cast<APawn>(SourceASC->GetAvatarActor());
			UE_LOG(LogTemp, Warning, TEXT("Source Character: %s"), *SourcePawn->GetActorNameOrLabel());
			APlayerState* SourcePlayerState = SourcePawn ? SourcePawn->GetPlayerState() : nullptr;
			DCData.ContributorPS = SourcePlayerState;

			AddDamageContributionData(DCData);
		}
	);
}

void UDamageContributionComponent::ProcessAllDatas()
{
	FNode* IterNode = DamageContributions.GetHead();

	while (!DamageContributions.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,TEXT("Recorded Damage: %f"), IterNode->GetValue().AppliedDamage);
		DamageContributions.RemoveNode(IterNode);
		IterNode = DamageContributions.GetHead();
	}
}

void UDamageContributionComponent::SpreadKillLogs(float MaxHealth)
{
	if (!GetOwner()->HasAuthority())
	{
		return;	
	}
	
	TMap<TWeakObjectPtr<APlayerState>,float> CalculatingContributionMap;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	for (const FDamageContributionData& ContributionData : DamageContributions)
	{
		if (!ContributionData.ContributorPS.IsValid())
			continue;

		if (CurrentTime - ContributionData.DamagedTime > CONTRIBUTION_TIME_LIMIT) //시간 차이를 초과했을 경우
			continue;

		//한 캐릭터의 기여도를 한 줄의 로그로 출력하기 위해 Map에 저장
		if (CalculatingContributionMap.Contains(ContributionData.ContributorPS))
		{
			CalculatingContributionMap[ContributionData.ContributorPS]+=ContributionData.AppliedDamage;
		}
		else
		{
			CalculatingContributionMap.Add(ContributionData.ContributorPS,ContributionData.AppliedDamage);
		}
	}
	
	for (TPair<TWeakObjectPtr<APlayerState>,float> ContributionData: CalculatingContributionMap)
	{
		if (!ContributionData.Key.IsValid())
			continue;
		
		AHPPlayerController* KillerPC = Cast<AHPPlayerController>(ContributionData.Key.Get()->GetPlayerController());

		if (KillerPC)
		{
			KillerPC->Client_OnReceiveEnemyEliminationInfo(FName(ContributionData.Key->GetPlayerName()),FMath::Min(ContributionData.Value,MaxHealth)/MaxHealth);
		}
	}
}


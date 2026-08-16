// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HPAbilitySystemComponent.h"

#include "GameplayEffectExtension.h"
#include "HPGameplayTags.h"
#include "AbilitySystem/HPAbilitySystemDataAsset.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Weapons/HPWeaponBase.h"

UHPAbilitySystemComponent::UHPAbilitySystemComponent()
{
	//GetGameplayAttributeValueChangeDelegate(UHPAttributeSet::GetHealthAttribute()).AddUObject(this, &UHPAbilitySystemComponent::OnHealthUpdate);
	//GetGameplayAttributeValueChangeDelegate(UHPAttributeSet::GetUltAttribute()).AddUObject(this, &UHPAbilitySystemComponent::OnUltUpdate);
}

void UHPAbilitySystemComponent::InitializeBaseAttributes()
{
	if (!HPAbilitySystemDataAsset|| !HPAbilitySystemDataAsset->GetBaseStatDataTable()|| !GetOwner())
	{
		return;
	}
	const UDataTable* BaseStatDataTable = HPAbilitySystemDataAsset->GetBaseStatDataTable();
	const FHPHeroBaseStats* HeroBaseStats = nullptr;

	for (const TPair<FName, uint8*>& DataPair : BaseStatDataTable->GetRowMap())
	{
		HeroBaseStats = BaseStatDataTable->FindRow<FHPHeroBaseStats>(DataPair.Key,"");
		if (HeroBaseStats && HeroBaseStats->Class == GetOwner()->GetClass())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is Selected"),*HeroBaseStats->Class->GetName());
			break;
		}
	}
	
	if (HeroBaseStats)
	{
		//NEXTTHINGTODO: HeroBaseStats살펴보고 더 채워넣기
		SetNumericAttributeBase(UHPAttributeSet::GetMaxHealthAttribute(), HeroBaseStats->BaseMaxHealth);
		SetNumericAttributeBase(UHPAttributeSet::GetMoveSpeedAttribute(), HeroBaseStats->BaseMoveSpeed);
		SetNumericAttributeBase(UHPAttributeSet::GetMaxUltAttribute(), HeroBaseStats->BaseMaxUlt);
		
	}
}

void UHPAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttributes();
	ApplyInitialEffects();
	GiveInitialAbilities();
	
	AbilitiesGiven= true;
	AbilitiesGivenDelegate.Broadcast();
}

void UHPAbilitySystemComponent::OnEquipWeapon(const AHPWeaponBase& NewWeapon)
{
	if (!GetOwner()||!GetOwner()->HasAuthority())
	{
		return;
	}
	
	for (const FGameplayAbilitySpecHandle& SpecHandle : GrantedWeaponAbilityHandles)
	{
		ClearAbility(SpecHandle);
	}
	GrantedWeaponAbilityHandles.Empty();
	
	FHPWeaponInfo WeaponInfo = NewWeapon.GetWeaponInfo();

	for (const TPair<EHPAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : WeaponInfo.WeaponAbilityMap)
	{
		FGameplayAbilitySpecHandle NewSpecHandle = GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key, nullptr));
		GrantedWeaponAbilityHandles.Add(NewSpecHandle);
	}

	FGameplayEffectSpecHandle InitBulletSpecHandle = MakeOutgoingSpec(WeaponInfo.WeaponInitBulletEffect, 1, MakeEffectContext());
	InitBulletSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("SetByCaller.MaxBullet"), WeaponInfo.MaxBullet);
	ApplyGameplayEffectSpecToSelf(*InitBulletSpecHandle.Data.Get());

	FGameplayEffectSpecHandle BulletChargeSpecHandle = MakeOutgoingSpec(WeaponInfo.WeaponBulletChargeEffect, 1, MakeEffectContext());
	ApplyGameplayEffectSpecToSelf(*BulletChargeSpecHandle.Data.Get());
}

ECombatState UHPAbilitySystemComponent::GetCombatState() const
{
	FHPGameplayTags GameplayTags = FHPGameplayTags::Get();

	if (HasMatchingGameplayTag(GameplayTags.State_Combat_Reloading))
	{
		return ECombatState::Reloading;
	}
	else if (HasMatchingGameplayTag(GameplayTags.State_Combat_SwappingWeapons))
	{
		return ECombatState::SwappingWeapons;
	}
	return ECombatState::Unoccupied;
}

void UHPAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!AbilitiesGiven)
	{
		AbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UHPAbilitySystemComponent::ForEachAbility(const FForEachAbilityDelegate& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

void UHPAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner()||!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!HPAbilitySystemDataAsset)
		return;
	for (const TSubclassOf<UGameplayEffect>& EffectClass : HPAbilitySystemDataAsset->GetInitialEffects())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(EffectClass,1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UHPAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner()||!GetOwner()->HasAuthority())
	{
		return;
	}

	for (const TPair<EHPAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key, nullptr));
	}

	for (const TPair<EHPAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key, nullptr));
	}

	for (const TSubclassOf<UGameplayAbility>& Ability : BasicAbilitiesWithNoKey)
	{
		GiveAbility(FGameplayAbilitySpec(Ability, 1, -1, nullptr));
	}
	if (!HPAbilitySystemDataAsset)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& Ability :HPAbilitySystemDataAsset->GetPassiveAbilities())
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1, -1, nullptr);
		GiveAbilityAndActivateOnce(AbilitySpec); //Passive Ability는 버튼이 필요없어 inputID는 1로 설정
	}
	for (const TSubclassOf<UGameplayAbility>& Ability :HPAbilitySystemDataAsset->GetPassiveAbilities_EventTriggered())
	{
		GiveAbility(FGameplayAbilitySpec(Ability, 1, -1, nullptr));
	}
}

void UHPAbilitySystemComponent::InitAbilityAndEffectAtRespawn()
{
	ApplyInitialEffects();

	for (const TSubclassOf<UGameplayAbility>& Ability :HPAbilitySystemDataAsset->GetPassiveAbilities())
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1, -1, nullptr);
		GiveAbilityAndActivateOnce(AbilitySpec); //Passive Ability는 버튼이 필요없어 inputID는 1로 설정
	}
	
}

void UHPAbilitySystemComponent::ApplyFullStatEffect()
{
	if (!HPAbilitySystemDataAsset)
	{
		return;
	}

	AuthApplyGameplayEffect(HPAbilitySystemDataAsset->GetFullStatEffect());
}

void UHPAbilitySystemComponent::AuthApplyGameplayEffect(const TSubclassOf<UGameplayEffect>& GameplayEffect, int32 Level)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(GameplayEffect, Level,MakeEffectContext());
	ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
//
// void UHPAbilitySystemComponent::OnHealthUpdate(const FOnAttributeChangeData& ChangedData)
// {
// 	if (!GetOwner() || !GetOwner()->HasAuthority())
// 	{
// 		return;
// 	}
//
// 	bool bFound = false;
//
// 	float MaxHealth = GetGameplayAttributeValue(UHPAttributeSet::GetMaxHealthAttribute(), bFound);
//
// 	if (bFound && ChangedData.NewValue >= MaxHealth)
// 	{
// 		if (!HasMatchingGameplayTag(FHPGameplayTags::Get().State_Health_Full))
// 		{
// 			AddLooseGameplayTag(FHPGameplayTags::Get().State_Health_Full);
// 		}
// 		else
// 		{
// 			RemoveLooseGameplayTag(FHPGameplayTags::Get().State_Health_Full);
// 		}
//
// 		if (ChangedData.NewValue <= 0)
// 		{
// 			if (!HasMatchingGameplayTag(FHPGameplayTags::Get().State_Health_Empty))
// 			{
// 				AddLooseGameplayTag(FHPGameplayTags::Get().State_Health_Empty);
//
// 			
// 				if(HPAbilitySystemDataAsset && HPAbilitySystemDataAsset->GetDeathEffect())
// 					AuthApplyGameplayEffect(HPAbilitySystemDataAsset->GetDeathEffect());
//
// 				FGameplayEventData DeadAbilityEventData;
// 				if(ChangedData.GEModData)
// 					DeadAbilityEventData.ContextHandle = ChangedData.GEModData->EffectSpec.GetContext();
//
// 				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), FHPGameplayTags::Get().State_Dead, DeadAbilityEventData);
// 			}
// 		}
// 		else
// 		{
// 			RemoveLooseGameplayTag(FHPGameplayTags::Get().State_Health_Empty);
// 		}
// 	}
// }
//
// void UHPAbilitySystemComponent::OnUltUpdate(const FOnAttributeChangeData& ChangedData)
// {
// 	if (!GetOwner() || !GetOwner()->HasAuthority())
// 	{
// 		return;
// 	}
//
// 	bool bFound = false;
//
// 	float MaxUlt = GetGameplayAttributeValue(UHPAttributeSet::GetMaxUltAttribute(), bFound);
//
// 	if (bFound && ChangedData.NewValue >= MaxUlt)
// 	{
// 		if (!HasMatchingGameplayTag(FHPGameplayTags::Get().State_Ult_Full))
// 		{
// 			AddLooseGameplayTag(FHPGameplayTags::Get().State_Ult_Full);
// 		}
// 		else
// 		{
// 			RemoveLooseGameplayTag(FHPGameplayTags::Get().State_Ult_Full);
// 		}
//
// 		if (ChangedData.NewValue <= 0)
// 		{
// 			if (!HasMatchingGameplayTag(FHPGameplayTags::Get().State_Ult_Empty))
// 			{
// 				AddLooseGameplayTag(FHPGameplayTags::Get().State_Ult_Empty);
// 			}
// 		}
// 		else
// 		{
// 			RemoveLooseGameplayTag(FHPGameplayTags::Get().State_Ult_Empty);
// 		}
// 	}
// }

FGameplayTag UHPAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec) const
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Ability"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

int32 UHPAbilitySystemComponent::GetInputIDFromSpec(const FGameplayAbilitySpec& AbilitySpec) const
{
	return AbilitySpec.InputID;
}

void UHPAbilitySystemComponent::ApplyDeathEffect()
{
	if (HPAbilitySystemDataAsset && HPAbilitySystemDataAsset->GetDeathEffect())
	{
		AuthApplyGameplayEffect(HPAbilitySystemDataAsset->GetDeathEffect());
	}
}

FGameplayTag UHPAbilitySystemComponent::GetUltTagForCurrentCharacter()
{
	FScopedAbilityListLock ActiveScopeLock(*this);

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability)
		{
			for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
			{
				if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Ability.Ult"))))
				{
					return Tag;
				}
			}
		}
	}
	return FGameplayTag();
}


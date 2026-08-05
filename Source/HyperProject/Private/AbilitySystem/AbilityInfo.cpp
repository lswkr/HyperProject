// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityInfo.h"

FHPAbilityInfo UAbilityInfo::FindAbilityInfoByTag(const FGameplayTag& AbilityTag)
{
	for (const FHPAbilityInfo& Info : AbilityInformation)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			UE_LOG(LogTemp, Warning, TEXT("FindAbilityInfoByTag-FoundTag: %s"), *AbilityTag.ToString());
			return Info;
		}
	}
	return FHPAbilityInfo();
}

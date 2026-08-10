// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HPGameplayAbilityTypes.h"

FHPHeroBaseStats::FHPHeroBaseStats():Class(nullptr),BaseMaxHealth(0.f),BaseMoveSpeed(0.f), BaseAttackDamage(0.f)
{}

bool FGameplayAbilityTargetData_HPCustom::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bool bParentOutSuccess = false;

	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bParentOutSuccess);

	uint8 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (!StartPoint.IsZero())
		{
			RepBits |= 1 << 0;
		}
		if (HitTime>0.f)
		{
			RepBits |= 1 << 1;
		}
		if (AimingDuration>0.f)
		{
			RepBits |= 1 << 2;
		}
		if (bIsNanoBoosted)
		{
			RepBits |= 1 << 3;
		}
		if (bIsAiming)
		{
			RepBits |= 1 << 4;
		}
	}
	Ar.SerializeBits(&RepBits, 5);

	if (RepBits & (1 << 0))
	{
		Ar << StartPoint;
	}
	if (RepBits & (1 << 1))
	{
		Ar << HitTime;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AimingDuration;
	}
	if (RepBits & (1 << 3))
	{
		Ar << bIsNanoBoosted;
	}
	if (RepBits & (1 << 4))
	{
		Ar << bIsAiming;
	}
	
	bOutSuccess = bParentOutSuccess;
	return true;
}
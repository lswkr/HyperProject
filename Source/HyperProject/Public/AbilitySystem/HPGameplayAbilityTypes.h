// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "HPGameplayAbilityTypes.generated.h"

class UAbilitySystemComponent;

/* ENUM Begin */
UENUM(BlueprintType)
enum class EHPAbilityInputID : uint8
{
	None			UMETA(DisplayName = "None"),
	BasicAttack		UMETA(DisplayName = "Basic Attack"),
	Ability_E		UMETA(DisplayName = "Ability E"),
	Ability_L_Shift UMETA(DisplayName = "Ability LeftShift"),
	AbilityThree	UMETA(DisplayName = "Ability Three"),
	AbilityFour		UMETA(DisplayName = "Ability Four"),
	AbilityUlt		UMETA(DisplayName = "Ability Ult"),
	RightButton		UMETA(DisplayName = "Ability RB"),
	MeleeHit		UMETA(DisplayName = "Melee Hit"),
	Reload			UMETA(DisplayName = "Reloading"),
	Confirm			UMETA(DisplayName = "Confirm"),
	Cancel			UMETA(DisplayName = "Cancel"),
	
};

UENUM(BlueprintType)
enum class EProjectileSpawnSocketType : uint8
{
	Weapon	UMETA(DisplayName = "Weapon"),
	Hand	UMETA(DisplayName = "Hand"),
};
/* ENUM End */


/* STRUCT Begin */
USTRUCT(BlueprintType)
struct FHPHeroBaseStats : public FTableRowBase
{
	GENERATED_BODY()

public:
	FHPHeroBaseStats();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;
	
	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseMaxUlt;
	
	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;

	UPROPERTY(EditAnywhere)
	float BaseAttackDamage;
	
};

USTRUCT(BlueprintType)
struct FProjectileParams
{
	GENERATED_BODY()

	FProjectileParams(){};

	UPROPERTY(BlueprintReadWrite)
	UObject* WorldContextObject = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UAbilitySystemComponent* SourceASC = nullptr;
};


USTRUCT(BlueprintType)
struct FHPControlPointData 
{
	GENERATED_BODY()

public:
	FHPControlPointData() {};

	UPROPERTY()
	int32 TeamOneCount = 0.f;
	UPROPERTY()
	int32 TeamTwoCount = 0.f;

	UPROPERTY()
	float TeamOneFightingPercent = 0.f;
	UPROPERTY()
	float TeamTwoFightingPercent = 0.f;

	UPROPERTY()
	float TeamOneCapturingPercent = 0.f;
	UPROPERTY()
	float TeamTwoCapturingPercent = 0.f;
};

USTRUCT()
struct FGamePlayTargetData_HPCustom : public FGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHeadShot = false;

	UPROPERTY()
	bool bAiming = false;

	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		bool bParentOutSuccess = false;

		FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bParentOutSuccess);

		uint8 RepBits = 0;
		if (Ar.IsSaving())
		{
			if (bHeadShot)
			{
				RepBits |= 1 << 0;
			}
			if (bAiming)
			{
				RepBits |= 1 << 1;
			}
		}
		Ar.SerializeBits(&RepBits, 2);

		if (RepBits & (1 << 0))
		{
			Ar << bHeadShot;
		}
		if (RepBits & (1 << 1))
		{
			Ar << bAiming;
		}
	
		bOutSuccess = bParentOutSuccess;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FGamePlayTargetData_HPCustom>
	: public TStructOpsTypeTraitsBase2<FGamePlayTargetData_HPCustom>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

/* STRUCT End */


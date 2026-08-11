// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "HPGameplayAbilityTypes.generated.h"

class AAbilitySpawnableActor;
class AHPPlayerCharacter;
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

UENUM(BlueprintType)
enum class EProjectileEffectType : uint8
{
	None		UMETA(DisplayName = "None"),
	EnemyOnly	UMETA(DisplayName = "Enemy Only"),
	TeamOnly	UMETA(DisplayName = "Team Only"),
	TeamAndEnemy UMETA(DisplayName = "Team And Enemy")
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
	TSubclassOf<AActor> Class = nullptr;
	
	UPROPERTY(EditAnywhere)
	float BaseMaxHealth = 0.0f;

	UPROPERTY(EditAnywhere)
	float BaseMaxUlt = 0.0f;
	
	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed = 0.0f;

	UPROPERTY(EditAnywhere)
	float BaseAttackDamage = 0.0f;
	
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
struct FGameplayAbilityTargetData_HPCustom : public FGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()
	
	FGameplayAbilityTargetData_HPCustom(){};
	FGameplayAbilityTargetData_HPCustom(FHitResult InHitResult, FVector InStartPoint, float InHitTime, float InAimingDuration, bool bInNanoBoosted, bool  bInAiming)
		:FGameplayAbilityTargetData_SingleTargetHit(InHitResult), StartPoint(InStartPoint), HitTime(InHitTime), AimingDuration(InAimingDuration),
	bIsNanoBoosted(bInNanoBoosted), bIsAiming(bInAiming){};
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

	FORCEINLINE FVector GetStartPoint() const { return StartPoint; }
	FORCEINLINE float GetHitTime() const { return HitTime; }
	FORCEINLINE float GetAimingDuration() const { return AimingDuration; }
	FORCEINLINE bool GetNanoBoosted() const { return bIsNanoBoosted; }
	FORCEINLINE bool IsAiming() const { return bIsAiming; }
	
private:
	UPROPERTY()
	FVector StartPoint = FVector::ZeroVector;

	UPROPERTY()
	float HitTime = 0.f;

	UPROPERTY()
	float AimingDuration = 0.f;
	
	UPROPERTY()
	bool bIsNanoBoosted = false;

	UPROPERTY()
	bool bIsAiming = false;

	

};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_HPCustom>
	: public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_HPCustom>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

USTRUCT(BlueprintType)
struct FProjectileApplyEffectParams
{
	GENERATED_BODY()

	UPROPERTY()
	AHPPlayerCharacter* SourceCharacter = nullptr;

	UPROPERTY()
	AHPPlayerCharacter* TargetCharacter = nullptr;
	
	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;
	
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> AdditionalEffectClass = nullptr;

	UPROPERTY()
	FVector OriginLocation = FVector::ZeroVector;
	
	UPROPERTY()
	float Damage = 0.f;

	UPROPERTY()
	float AdditionalEffectValue = 0.f;

	UPROPERTY()
	bool bCanHeadShot = false;

	UPROPERTY()
	bool bCanPush = false;

	UPROPERTY()
	bool bDistanceFalloff = false;

	UPROPERTY()
	bool bAdditionalEffectForTeam = false;

	UPROPERTY()
	float PushPower = 0.0f;
	
	UPROPERTY()
	float InnerRadius = 0.0f;

	UPROPERTY()
	float OuterRadius = 0.0f;

	UPROPERTY()
	EProjectileEffectType EffectType = EProjectileEffectType::None;

	UPROPERTY()
	FGenericTeamId GenericTeamId;

	UPROPERTY()
	TSubclassOf<AAbilitySpawnableActor> SpawnableActorClass;
};

/* STRUCT End */


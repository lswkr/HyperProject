// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HPGameplayAbilityTypes.h"
#include "Engine/DataAsset.h"
#include "AbilityInfo.generated.h"

/**
 * 
 */

enum class EHPAbilityInputID : uint8;

USTRUCT(BlueprintType)
struct FHPAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	FGameplayTag AbilityTag = FGameplayTag();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	EHPAbilityInputID InputID = EHPAbilityInputID::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag = FGameplayTag();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
};

UCLASS()
class HYPERPROJECT_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	FHPAbilityInfo FindAbilityInfoByTag(const FGameplayTag& AbilityTag);

private:
	UPROPERTY(EditDefaultsOnly, Category = "AbilityInformation")
	TArray<FHPAbilityInfo> AbilityInformation;
};

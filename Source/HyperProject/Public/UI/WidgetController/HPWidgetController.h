// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HPWidgetController.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedDelegate, float, NewValue);

class UAttributeSet;
class UAbilitySystemComponent;
class UHPCombatComponent;
class AHPPlayerController;
class UHPAttributeSet;
class UHPAbilitySystemComponent;
class UDamageContributionComponent;

USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams() {}
	FWidgetControllerParams(APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS, UHPCombatComponent* CC/*, UDamageContributionComponent* DC*/)
	: PlayerController(PC), AbilitySystemComponent(ASC), AttributeSet(AS), CombatComponent(CC)/*, DamageContributionComponent(DC)*/ {}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APlayerController* PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAttributeSet* AttributeSet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHPCombatComponent* CombatComponent = nullptr;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// UDamageContributionComponent* DamageContributionComponent = nullptr;
};

UCLASS()
class HYPERPROJECT_API UHPWidgetController : public UObject
{
	GENERATED_BODY()

public:
	void SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams);
	virtual void BindCallbacksToDependencies();
	virtual void BroadcastInitialValues();
	
protected:
	//NEXTTHINGTODO: UPROPERTY 정리하기
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController|Data")
	APlayerController* PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController|Data")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController|Data")
	UAttributeSet* AttributeSet;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController|Data")
	AHPPlayerController* HPPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController|Data")
	UHPAbilitySystemComponent* HPAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController|Data")
	UHPAttributeSet* HPAttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController|Data")
	UHPCombatComponent* HPCombatComponent;
	//
	// UPROPERTY(BlueprintReadOnly, Category = "WidgetController|Data")
	// UDamageContributionComponent* DamageContributionComponent;

	AHPPlayerController* GetHPPlayerController();
	UHPAbilitySystemComponent* GetHPAbilitySystemComponent();
	UHPAttributeSet* GetHPAttributeSet();
	UHPCombatComponent* GetHPCombatComponent() const;
	//UDamageContributionComponent* GetDamageContributionComponent() const;
};

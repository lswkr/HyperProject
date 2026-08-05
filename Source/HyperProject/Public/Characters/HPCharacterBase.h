// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"

#include "HPCharacterBase.generated.h"

class UHPAbilitySystemComponent;
class UHPAttributeSet;
class UAttributeSet;
class UHPCombatComponent;

UCLASS()
class HYPERPROJECT_API AHPCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHPCharacterBase();
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void ServerSideInit();
	virtual void ClientSideInit();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAttributeSet* GetAttributeSet() const;
	
	UFUNCTION(BlueprintPure)
	UHPCombatComponent* GetCombatComponent() const ;
	
protected:
	virtual void PossessedBy(AController* NewController) override;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "GAS") //Weapon바꿀 때 PlayerCharacter클래스에서 UHPAbilitySystemComponent의 함수를 사용하기 위해 protected
	UHPAbilitySystemComponent* HPAbilitySystemComponent;

	UPROPERTY()
	UHPAttributeSet* HPAttributeSet;

	UPROPERTY(VisibleDefaultsOnly, Category = "Combat")
	UHPCombatComponent* CombatComponent;
	
private:

};

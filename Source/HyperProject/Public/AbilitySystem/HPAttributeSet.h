// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "HPAttributeSet.generated.h"

/**
 * 
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(){}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

UCLASS()
class HYPERPROJECT_API UHPAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,MaxHealth)
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,Health)
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,MaxUlt)
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,Ult)
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,MaxBullet)
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,Bullet)
	
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,MoveSpeed)
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,IncomingDamage)
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,IncomingHeal)
	ATTRIBUTE_ACCESSORS(UHPAttributeSet,IncomingUlt)
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;


private:
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;

	UPROPERTY(ReplicatedUsing = OnRep_MaxUlt)
	FGameplayAttributeData MaxUlt;
	
	UPROPERTY(ReplicatedUsing = OnRep_Ult)
	FGameplayAttributeData Ult;

	UPROPERTY(ReplicatedUsing = OnRep_MaxBullet)
	FGameplayAttributeData MaxBullet;
	
	UPROPERTY(ReplicatedUsing = OnRep_Bullet)
	FGameplayAttributeData Bullet;

	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;

	UPROPERTY(ReplicatedUsing = OnRep_CrouchSpeed)
	FGameplayAttributeData CrouchSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingHeal;
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingUlt;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxUlt(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Ult(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxBullet(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Bullet(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CrouchSpeed(const FGameplayAttributeData& OldValue);

	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& OutProps) const;
	void HandleIncomingDamage(const FEffectProperties& Props);
	void HandleUlt(const FEffectProperties& Props);
	void HandleIncomingHeal(const FEffectProperties& Props);
	void SendUltEvent(const FEffectProperties& Props, float UltPoint);
};

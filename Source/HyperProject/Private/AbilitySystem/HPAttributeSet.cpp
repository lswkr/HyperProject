// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HPAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "HPGameplayTags.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "GameFramework/Character.h"

void UHPAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHPAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHPAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHPAttributeSet, MaxUlt, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHPAttributeSet, Ult, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHPAttributeSet, MaxBullet, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHPAttributeSet, Bullet, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHPAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHPAttributeSet, CrouchSpeed, COND_None, REPNOTIFY_Always);
}

void UHPAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
	}

	if (Attribute == GetUltAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxUlt());
	}

	if (Attribute == GetBulletAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxBullet());
	}
}

void UHPAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);
	
	const FHPGameplayTags& GameplayTags = FHPGameplayTags::Get();

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}

	if (Data.EvaluatedData.Attribute == GetUltAttribute())
	{
		HandleUlt(Props);
		//SetUlt(FMath::Clamp(GetUlt(), 0.f, GetMaxUlt()));
	}

	if (Data.EvaluatedData.Attribute == GetBulletAttribute())
	{
		SetBullet(FMath::Clamp(GetBullet(), 0.f, GetMaxBullet()));
	}

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props);
	}

	if (Data.EvaluatedData.Attribute == GetIncomingHealAttribute())
	{
		HandleIncomingHeal(Props);
	}
}


void UHPAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHPAttributeSet, Health, OldValue);
}

void UHPAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHPAttributeSet, MaxHealth, OldValue);
}

void UHPAttributeSet::OnRep_Ult(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHPAttributeSet, Ult, OldValue);
}

void UHPAttributeSet::OnRep_MaxBullet(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHPAttributeSet, MaxBullet, OldValue);
}

void UHPAttributeSet::OnRep_Bullet(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHPAttributeSet, Bullet, OldValue);
}

void UHPAttributeSet::OnRep_MaxUlt(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHPAttributeSet, MaxUlt, OldValue);
}

void UHPAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHPAttributeSet, MoveSpeed, OldValue);
}

void UHPAttributeSet::OnRep_CrouchSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHPAttributeSet, CrouchSpeed, OldValue);
}

void UHPAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& OutProps) const
{
	OutProps.EffectContextHandle = Data.EffectSpec.GetContext();
	OutProps.SourceASC = OutProps.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(OutProps.SourceASC) && OutProps.SourceASC->AbilityActorInfo.IsValid() && OutProps.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		OutProps.SourceAvatarActor = OutProps.SourceASC->AbilityActorInfo->AvatarActor.Get();
		OutProps.SourceController = OutProps.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (OutProps.SourceController == nullptr && OutProps.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(OutProps.SourceAvatarActor))
			{
				OutProps.SourceController = Pawn->GetController();
			}
		}
		if (OutProps.SourceController)
		{
			OutProps.SourceCharacter = Cast<ACharacter>(OutProps.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		OutProps.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		OutProps.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		OutProps.TargetCharacter = Cast<ACharacter>(OutProps.TargetAvatarActor);
		OutProps.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OutProps.TargetAvatarActor);
	}
}

void UHPAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0.f);
	
	if (LocalIncomingDamage > 0.f) //힐도 궁극기에 영향 주기 위해 절댓값
	{
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		const float NewUltPoint = GetHealth()>LocalIncomingDamage ? LocalIncomingDamage : GetHealth();
		
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		SendUltEvent(Props, NewUltPoint);
		
		//ASC의 상태에 따라(궁극기 중에는 안 차도록)
		
		
		const bool bFatal = NewHealth <= 0.f;
		if (bFatal)
		{
			AHPPlayerCharacter* PlayerCharacter = Cast<AHPPlayerCharacter>(Props.TargetCharacter);
			PlayerCharacter->Death();
			
		}
		else
		{
			// if (Props.TargetCharacter->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsBeingShocked(Props.TargetCharacter))
			// {
			// 	FGameplayTagContainer TagContainer;
			// 	TagContainer.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
			// 	Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
			// }
			//
			// const FVector& KnockbackForce = UAuraAbilitySystemLibrary::GetKnockbackForce(Props.EffectContextHandle);
			// if (!KnockbackForce.IsNearlyZero(1.f))
			// {
			// 	Props.TargetCharacter->LaunchCharacter(KnockbackForce, true, true);
			// }
		}
			
		// const bool bBlock = UAuraAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
		// const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
		// ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCriticalHit);
		// if (UAuraAbilitySystemLibrary::IsSuccessfulDebuff(Props.EffectContextHandle))
		// {
		// 	Debuff(Props);
		// }
	}
}

void UHPAttributeSet::HandleIncomingHeal(const FEffectProperties& Props)
{
	const float LocalIncomingHeal = GetIncomingHeal();
	SetIncomingHeal(0.f);
	
	if (LocalIncomingHeal > 0.f) //힐도 궁극기에 영향 주기 위해 절댓값
	{
		const float NewHealth = GetHealth() + LocalIncomingHeal;
		const float NewUltPoint = GetHealth()>LocalIncomingHeal ? LocalIncomingHeal : GetHealth();
		
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		SendUltEvent(Props, NewUltPoint);
		
		//NEXTTHINGTODO: ASC의 상태에 따라(궁극기 중에는 안 차도록)

		
			// if (Props.TargetCharacter->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsBeingShocked(Props.TargetCharacter))
			// {
			// 	FGameplayTagContainer TagContainer;
			// 	TagContainer.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
			// 	Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
			// }
			//
			// const FVector& KnockbackForce = UAuraAbilitySystemLibrary::GetKnockbackForce(Props.EffectContextHandle);
			// if (!KnockbackForce.IsNearlyZero(1.f))
			// {
			// 	Props.TargetCharacter->LaunchCharacter(KnockbackForce, true, true);
			// }
		
			
		// const bool bBlock = UAuraAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
		// const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
		// ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCriticalHit);
		// if (UAuraAbilitySystemLibrary::IsSuccessfulDebuff(Props.EffectContextHandle))
		// {
		// 	Debuff(Props);
		// }
	}
}
void UHPAttributeSet::HandleUlt(const FEffectProperties& Props)
{
	SetUlt(FMath::Clamp(GetUlt(), 0.f, GetMaxUlt()));

	UE_LOG(LogTemp, Warning, TEXT("NewUlt: %f"), GetUlt());
	
	if (GetUlt() >=GetMaxUlt())
	{
		UE_LOG(LogTemp, Warning, TEXT("UltFull"));

		if (UAbilitySystemComponent* ASC = Props.TargetASC)
		{
			const FHPGameplayTags& GameplayTags = FHPGameplayTags::Get();
			if (ASC->HasMatchingGameplayTag(GameplayTags.State_Ult_Full))
				return;
			
			UE_LOG(LogTemp, Warning, TEXT("UltFull Applied"));

			FGameplayEventData Payload;
			Payload.EventTag = GameplayTags.State_Ult_Full;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.TargetAvatarActor, GameplayTags.Event_ListenFor_Ult_Full,Payload);
		}
	}
	
}

void UHPAttributeSet::SendUltEvent(const FEffectProperties& Props, float UltPoint)
{
	if (AHPPlayerCharacter* SourcePlayerCharacter = Cast<AHPPlayerCharacter>(Props.SourceCharacter))
	{
		const FHPGameplayTags& GameplayTags = FHPGameplayTags::Get();
		FGameplayEventData Payload;
		Payload.EventTag = GameplayTags.Attribute_Meta_IncomingUlt;
		Payload.EventMagnitude = UltPoint;
		UE_LOG(LogTemp,Warning,TEXT("ULT: %f"), UltPoint);
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attribute_Meta_IncomingUlt, Payload);
	}
}
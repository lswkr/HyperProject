// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FHPGameplayTags
{
public:
	static const FHPGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	/*AbilityTag Begin*/
	FGameplayTag Ability_Reload;
	FGameplayTag Ability_Aiming;
	FGameplayTag Ability_Reload_ApplyEffect;
	FGameplayTag Ability_Attack_Murdock_BasicAttack;
	FGameplayTag Ability_Attack_Murdock_Mine;
	FGameplayTag Ability_Attack_Belica_BasicAttack;
	FGameplayTag Ability_Attack_Belica_HealBan;
	FGameplayTag Ability_Ult_Belica_NanoBoost;
	FGameplayTag Ability_Ult_Murdock_GravitonSurge;
	FGameplayTag Ability_Attack_MeleeHit;
	FGameplayTag Ability_HitReact;
	
	/*AbilityTag End*/

	/*State Begin*/
	FGameplayTag State_Health_Full;
	FGameplayTag State_Health_Empty;
	
	FGameplayTag State_Ult_Full;
	FGameplayTag State_Ult_Empty;

	FGameplayTag State_Bullet_Full;
	FGameplayTag State_Bullet_Empty;
	
	FGameplayTag State_Dead;
	
	FGameplayTag State_Debuff_HealBan;
	
	FGameplayTag State_Combat_Unoccupied;
	FGameplayTag State_Combat_Reloading;
	FGameplayTag State_Combat_SwappingWeapons;

	FGameplayTag State_Combat_NanoBoosted;
	
	FGameplayTag State_Combat_Aiming;
	
	FGameplayTag State_Using_Ult;
	/*State End*/
	
	/*SetByCaller Begin*/
	FGameplayTag SetByCaller_MaxBullet;
	FGameplayTag SetByCaller_BulletsPerShot;
	FGameplayTag SetByCaller_IncomingDamage;
	FGameplayTag SetByCaller_IncomingHeal;
	FGameplayTag SetByCaller_IncomingUlt;
	/*SetByCaller End*/
	
	/*Events Begin*/
	FGameplayTag Event_Reload;
	FGameplayTag Event_Reload_ApplyEffect;
	FGameplayTag Event_MeleeHit;
	FGameplayTag Event_HitReact;
	FGameplayTag Event_ZoomIn;
	FGameplayTag Event_ZoomOut;
	FGameplayTag Event_SpawnProjectile;
	FGameplayTag Event_ListenFor_Ult_Full;
	FGameplayTag Event_Belica_NanoBoost;
	/*Events End*/

	/* ApplicationTag Begin*/
	FGameplayTag ApplicationTag_Normal;
	FGameplayTag ApplicationTag_Aiming;
	/* ApplicationTag End*/

	/* Cooldown Begin */
	FGameplayTag Cooldown_Ability_Mine;
	FGameplayTag Cooldown_Ability_HealBan;
	
	/* Cooldown End */

	/* Attributes Begin*/
	FGameplayTag Attribute_Meta_IncomingUlt;
	
	/* Attributes End*/
	/* etc */
	FGameplayTag Remove_SetMine;
	
private:
	static FHPGameplayTags GameplayTags;

};

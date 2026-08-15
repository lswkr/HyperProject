// Fill out your copyright notice in the Description page of Project Settings.


#include "HPGameplayTags.h"

#include "GameplayTagsManager.h"

FHPGameplayTags FHPGameplayTags::GameplayTags;

void FHPGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.Ability_Reload = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Reload"),
	FString("Tag for Reload")
	);

	GameplayTags.Ability_Reload_ApplyEffect = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Reload.ApplyEffect"),
	FString("Tag for Applying Reloading Effect")
	);

	GameplayTags.Ability_Aiming = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Aiming"),
	FString("Tag for Aiming")
	);
	
	GameplayTags.Ability_Attack_Murdock_BasicAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Attack.Murdock.BasicAttack"),
	FString("Tag for Murdock's BasicAttack")
	);

	GameplayTags.Ability_Attack_Murdock_Mine = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Attack.Murdock.Mine"),
	FString("Tag for Murdock's Mine")
	);
	
	GameplayTags.Ability_Attack_Belica_BasicAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Attack.Belica.BasicAttack"),
	FString("Tag for Belica's BasicAttack")
	);

	GameplayTags.Ability_Attack_Belica_HealBan = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Attack.Belica.HealBan"),
	FString("Tag for Belica's HealBan")
	);

	GameplayTags.Ability_Ult_Belica_NanoBoost = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Ult.Belica.NanoBoost"),
	FString("Tag for Belica's NanoBoost")
	);

	GameplayTags.Ability_Ult_Murdock_GravitonSurge = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Ult.Murdock.GravitonSurge"),
	FString("Tag for Murdock's GravitonSurge")
	);

	GameplayTags.Ability_Attack_MeleeHit = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.Attack.MeleeHit"),
	FString("Tag for melee hit ability")
	);

	GameplayTags.Ability_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Ability.HitReact"),
	FString("Tag for hit react")
	);
	
	GameplayTags.State_Health_Full = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Health.Max"),
	FString("Max Health State")
	);

	GameplayTags.State_Health_Empty = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Health.Empty"),
	FString("Empty Health State")
	);

	GameplayTags.State_Ult_Full = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Ult.Full"),
	FString("Full Ult State")
	);

	GameplayTags.State_Ult_Empty = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Ult.Empty"),
	FString("Empty Ult State")
	);

	GameplayTags.State_Bullet_Full = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Bullet.Full"),
	FString("Empty Ult State")
	);

	GameplayTags.State_Bullet_Empty = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Bullet.Empty"),
	FString("Empty Ult State")
	);
	
	GameplayTags.State_Dead = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Dead"),
	FString("Dead State")
	);

	GameplayTags.State_Debuff_HealBan = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Debuff.HealBan"),
	FString("Heal Ban State")
	);

	GameplayTags.State_Combat_Unoccupied = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Combat.Unoccupied"),
	FString("Unoccupied while in combat")
	);

	GameplayTags.State_Combat_Reloading = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Combat.Reloading"),
	FString("State while reloading")
	);

	GameplayTags.State_Combat_SwappingWeapons = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Stat.Combat.SwappingWeapons"),
	FString("State while swapping weapons")
	);

	GameplayTags.State_Combat_Aiming = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Combat.Aiming"),
	FString("State while Aiming")
	);

	GameplayTags.State_Combat_NanoBoosted = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Combat.NanoBoosted"),
	FString("NanoBoosted State")
	);
	
	GameplayTags.State_Using_Ult = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("State.Using.Ult"),
	FString("State while using ult. It is used to blocks ult point.")
	);

	GameplayTags.SetByCaller_MaxBullet = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("SetByCaller.MaxBullet"),
	FString("Max Bullet Tag for SetByCaller")
	);

	GameplayTags.SetByCaller_BulletsPerShot = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("SetByCaller.BulletsPerShot"),
	FString("Bullets per shot Tag for SetByCaller")
	);
	GameplayTags.SetByCaller_IncomingDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("SetByCaller.IncomingDamage"),
	FString("Incoming Damage Tag for SetByCaller")
	);

	GameplayTags.SetByCaller_IncomingHeal = UGameplayTagsManager::Get().AddNativeGameplayTag(
FName("SetByCaller.IncomingHeal"),
FString("Incoming Heal Tag for SetByCaller")
);
	GameplayTags.SetByCaller_IncomingUlt = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("SetByCaller.IncomingUlt"),
	FString("Incoming Ult Tag for SetByCaller")
	);


	GameplayTags.Event_Reload = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Event.Reload"),
	FString("Tag that trigger Reloading Ability")
	);

	GameplayTags.Event_Reload_ApplyEffect = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Event.Reload.ApplyEffect"),
	FString("Tag that trigger Applying Reloading Effect Ability")
	);

	GameplayTags.Event_MeleeHit = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Event.MeleeHit"),
	FString("Tag sent to owner when target get melee hit")
	);

	GameplayTags.Event_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Event.HitReact"),
	FString("Tag sent to owner when target get melee hit")
	);

	GameplayTags.Event_ZoomIn = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Event.ZoomIn"),
	FString("Tag sent to owner when aiming start")
	);

	GameplayTags.Event_ZoomOut = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Event.ZoomOut"),
	FString("Tag sent to owner when aiming end")
	);

	GameplayTags.Event_SpawnProjectile = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Event.SpawnProjectile"),
	FString("Tag sent to owner when spawn projectile animation notify")
	);
	
	GameplayTags.Event_ListenFor_Ult_Full = UGameplayTagsManager::Get().AddNativeGameplayTag(
    	FName("Event.ListenFor.Ult.Full"),
    	FString("Tag sent to owner when ult gauge be full")
    	);

	GameplayTags.Event_Belica_NanoBoost = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Event.Belica.NanoBoost"),
	FString("Tag sent to owner when target get melee hit")
	);

	GameplayTags.ApplicationTag_Normal = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("ApplicationTag.Normal"),
	FString("Confirming normal state tag when send target data")
	);

	GameplayTags.ApplicationTag_Aiming = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("ApplicationTag.Aiming"),
	FString("Confirming aiming state tag when send target data")
	);

	GameplayTags.Cooldown_Ability_Mine = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Cooldown.Ability.Mine"),
	FString("Cooldown tag for mine ability")
	);

	GameplayTags.Cooldown_Ability_HealBan= UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Cooldown.Ability.HealBan"),
	FString("Cooldown tag for HealBan ability")
	);
	
	GameplayTags.Attribute_Meta_IncomingUlt = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Attribute.Meta.IncomingUlt"),
	FString("IncomingUlt tag for Listen ult event")
	);
	
	GameplayTags.Remove_SetMine = UGameplayTagsManager::Get().AddNativeGameplayTag(
    	FName("Remove.SetMine"),
    	FString("Tag for removing set mine")
    	);

	
}

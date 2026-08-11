// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PDA_CharacterDefinition.h"

#include "Characters/Player/HPPlayerCharacter.h"

FPrimaryAssetId UPDA_CharacterDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetCharacterDefinitionAssetType(),GetFName());
}

FPrimaryAssetType UPDA_CharacterDefinition::GetCharacterDefinitionAssetType()
{
	return FPrimaryAssetType("CharacterDefinition");
}

UTexture2D* UPDA_CharacterDefinition::LoadIcon() const
{
	CharacterIcon.LoadSynchronous();

	if (CharacterIcon.IsValid())
		return CharacterIcon.Get();

	return nullptr;
}

TSubclassOf<AHPPlayerCharacter> UPDA_CharacterDefinition::LoadCharacterClass() const
{
	CharacterClass.LoadSynchronous();
	if (CharacterClass.IsValid())
		return CharacterClass.Get();

	return TSubclassOf<AHPPlayerCharacter>();
}

TSubclassOf<UAnimInstance> UPDA_CharacterDefinition::LoadDisplayAnimInstance() const
{
	DisplayAnimClass.LoadSynchronous();
	if (DisplayAnimClass.IsValid())
		return DisplayAnimClass.Get();

	return TSubclassOf<UAnimInstance>();
}

USkeletalMesh* UPDA_CharacterDefinition::LoadDisplaySkeletalMesh() const
{
	TSubclassOf<AHPPlayerCharacter> LoadedCharacterClass = LoadCharacterClass();

	if (!LoadedCharacterClass)
		return nullptr;

	AHPPlayerCharacter* Character = LoadedCharacterClass.GetDefaultObject();

	if (!Character)
		return nullptr;
	
	return Character->GetMesh()->GetSkeletalMeshAsset();
}

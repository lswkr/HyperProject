// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PDA_CharacterDefinition.generated.h"

/**
 * 
 */
class AHPPlayerCharacter;

UCLASS()
class HYPERPROJECT_API UPDA_CharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetCharacterDefinitionAssetType();
	FString GetCharacterDisplayName() const {return CharacterName;}
	UTexture2D* LoadIcon() const;
	TSubclassOf<AHPPlayerCharacter> LoadCharacterClass() const;
	TSubclassOf<UAnimInstance> LoadDisplayAnimInstance() const;
	USkeletalMesh* LoadDisplaySkeletalMesh() const;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Character")
	FString CharacterName;
	
	UPROPERTY(EditDefaultsOnly, Category="Character")
	TSoftObjectPtr<UTexture2D> CharacterIcon;

	UPROPERTY(EditDefaultsOnly, Category="Character")
	TSoftClassPtr<AHPPlayerCharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly, Category="Character")
	TSoftClassPtr<UAnimInstance> DisplayAnimClass;
};

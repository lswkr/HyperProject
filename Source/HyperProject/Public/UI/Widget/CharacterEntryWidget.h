// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CharacterEntryWidget.generated.h"

/**
 * 
 */
class UImage;
class UTextBlock;
class UPDA_CharacterDefinition;

UCLASS()
class HYPERPROJECT_API UCharacterEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	FORCEINLINE const UPDA_CharacterDefinition* GetCharacterDefinition() const { return CharacterDefinition; }
	void SetSelected(bool bIsSelected);
	
private:
	UPROPERTY(meta = (BindWidget))
	UImage* Image_CharacterIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_CharacterName;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	FName IconTextureMatParamName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	FName SaturationMatParamName = "Saturation";

	UPROPERTY()
	const UPDA_CharacterDefinition* CharacterDefinition;
};

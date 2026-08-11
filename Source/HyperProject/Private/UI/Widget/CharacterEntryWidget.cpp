// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/CharacterEntryWidget.h"

#include "Characters/PDA_CharacterDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCharacterEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	CharacterDefinition = Cast<UPDA_CharacterDefinition>(ListItemObject);

	if (CharacterDefinition)
	{
		Image_CharacterIcon->GetDynamicMaterial()->SetTextureParameterValue(IconTextureMatParamName, CharacterDefinition->LoadIcon());
		TextBlock_CharacterName->SetText(FText::FromString(CharacterDefinition->GetCharacterDisplayName()));
	}
}

void UCharacterEntryWidget::SetSelected(bool bIsSelected)
{
	Image_CharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(SaturationMatParamName, bIsSelected ? 0.f : 1.f);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/PlayerTeamSlotWidget.h"

#include "Characters/PDA_CharacterDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPlayerTeamSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Image_CharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(CharacterEmptyMatParamName, 0);

	CachedCharacterNameString = "";
}

void UPlayerTeamSlotWidget::UpdateSlot(const FString& PlayerName, const UPDA_CharacterDefinition* CharacterDefinition)
{
	CachedPlayerNameString = PlayerName;

	if (CharacterDefinition)
	{
		Image_CharacterIcon->GetDynamicMaterial()->SetTextureParameterValue(CharacterIconMatParamName, CharacterDefinition->LoadIcon());
		Image_CharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(CharacterEmptyMatParamName, 0);

		CachedCharacterNameString = CharacterDefinition->GetCharacterDisplayName();
	}
	else
	{
		Image_CharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(CharacterEmptyMatParamName, 1);

		CachedCharacterNameString = "";
	}

	UpdateNameText();
}

void UPlayerTeamSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	Text_Name->SetText(FText::FromString(CachedCharacterNameString));
	PlayAnimationForward(HoverAnim);
}

void UPlayerTeamSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	Text_Name->SetText(FText::FromString(CachedPlayerNameString));
	PlayAnimationReverse(HoverAnim);
}

void UPlayerTeamSlotWidget::UpdateNameText()
{
	if (IsHovered())
	{
		Text_Name->SetText(FText::FromString(CachedCharacterNameString));
	}
	else
	{
		Text_Name->SetText(FText::FromString(CachedPlayerNameString));
	}
}

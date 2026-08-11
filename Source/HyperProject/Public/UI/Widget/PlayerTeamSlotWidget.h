// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTeamSlotWidget.generated.h"

/**
 * 
 */

class UImage;
class UTextBlock;
class UPDA_CharacterDefinition;


UCLASS()
class HYPERPROJECT_API UPlayerTeamSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void UpdateSlot(const FString& PlayerName, const UPDA_CharacterDefinition* CharacterDefinition);
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* HoverAnim;
	
	UPROPERTY(meta=(BindWidget))
	UImage* Image_CharacterIcon;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* Text_Name;

	UPROPERTY(EditAnywhere, Category = "Visual")
	FName CharacterIconMatParamName = "Icon";

	UPROPERTY(EditAnywhere, Category = "Visual")
	FName CharacterEmptyMatParamName = "Empty";
	
	FString CachedPlayerNameString;
	FString CachedCharacterNameString;

	void UpdateNameText();
};

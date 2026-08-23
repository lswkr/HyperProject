// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionEntryWidget.generated.h"

class UTextBlock;
class UButton;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionEntrySelected, const FString& /*SelectedSessionIDStr*/)
/**
 * 
 */
UCLASS()
class HYPERPROJECT_API USessionEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	FOnSessionEntrySelected OnSessionEntrySelected;
	void InitializeEntry(const FString& Name, const FString& SessionIdStr);


	FORCEINLINE FString GetCachedSessionIdStr() const { return CachedSessionIdStr;  }
private:
	UPROPERTY(meta= (BindWidget))
	UTextBlock* TextBlock_SessionName;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Session;

	FString CachedSessionIdStr;

	UFUNCTION()
	void SessionEntrySelected();
};


// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/HPUserWidget.h"

void UHPUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}

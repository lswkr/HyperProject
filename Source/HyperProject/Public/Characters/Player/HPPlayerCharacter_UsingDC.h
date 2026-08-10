// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "HPPlayerCharacter_UsingDC.generated.h"

/**
 * 
 */
class UDetectComponent;

UCLASS()
class HYPERPROJECT_API AHPPlayerCharacter_UsingDC : public AHPPlayerCharacter
{
	GENERATED_BODY()
public:
	AHPPlayerCharacter_UsingDC();

	UFUNCTION(BlueprintCallable, Category = "DetectComponent")
	UDetectComponent* GetDetectComponent() const { return DetectComponent; }

protected:
	virtual void ClientSideInit() override;

private:
	UPROPERTY(EditDefaultsOnly)
	UDetectComponent* DetectComponent;
};

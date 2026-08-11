// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character_Display.generated.h"

class UCameraComponent;
class UPDA_CharacterDefinition;

UCLASS()
class HYPERPROJECT_API ACharacter_Display : public AActor
{
	GENERATED_BODY()
	
public:	
	ACharacter_Display();
	void ConfigureWithCharacterDefinition(const UPDA_CharacterDefinition* CharacterDefinition);

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Character Display")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Character Display")
	UCameraComponent* CameraComponent;
};

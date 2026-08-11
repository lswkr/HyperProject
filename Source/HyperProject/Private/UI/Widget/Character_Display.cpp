// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Character_Display.h"

#include "Camera/CameraComponent.h"
#include "Characters/PDA_CharacterDefinition.h"

ACharacter_Display::ACharacter_Display()
{
 	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComp")));

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent ->SetupAttachment(GetRootComponent());
}

void ACharacter_Display::ConfigureWithCharacterDefinition(const UPDA_CharacterDefinition* CharacterDefinition)
{
	if (!CharacterDefinition)
		return;

	MeshComponent->SetSkeletalMesh(CharacterDefinition->LoadDisplaySkeletalMesh());
	MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	MeshComponent->SetAnimInstanceClass(CharacterDefinition->LoadDisplayAnimInstance());
}




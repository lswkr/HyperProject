// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/HPPlayerCharacter_UsingDC.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "Components/DetectComponent.h"
#include "Controller/HPPlayerController.h"

AHPPlayerCharacter_UsingDC::AHPPlayerCharacter_UsingDC()
{
	DetectComponent = CreateDefaultSubobject<UDetectComponent>("DetectComponent");
	
}

void AHPPlayerCharacter_UsingDC::Client_OnSkillActivate_Implementation(FVector_NetQuantize100 TargetLocation)
{
	DetectComponent->EraseTargetLockWidget();

	if (ClientAnimMontage && BeamSystem)
	{
		
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(ClientAnimMontage);
		}
		if (GetWorld() && Implements<UCombatInterface>())
		{
			FVector BeamStart = Execute_GetUltMuzzleSocketLocation(this);
			FVector BeamEnd = TargetLocation;
			
			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BeamSystem, BeamStart);

			if (!NiagaraComponent)
				return;

			NiagaraComponent->SetVectorParameter(FName("Beam End"),BeamEnd);
		}
	}
}

void AHPPlayerCharacter_UsingDC::ClientSideInit()
{
	Super::ClientSideInit();

	if (AHPPlayerController* CurrentHPPlayerController = GetHPPlayerController())
	{
		DetectComponent->InitComponent(HPAbilitySystemComponent, CurrentHPPlayerController, this);
	}
	
}

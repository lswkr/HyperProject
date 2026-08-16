// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/GCNB_SFX_Local.h"

#include "Kismet/GameplayStatics.h"


bool UGCNB_SFX_Local::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	UE_LOG(LogTemp, Warning,TEXT("CueExecuted Target: %s, Instigator: %s"), *Target->GetName(), *Parameters.Instigator->GetName());
	if (Parameters.IsInstigatorLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("LocallyExecuted"));
		UGameplayStatics::PlaySoundAtLocation(
		   Target,
		   SoundEffect,
		   Parameters.Location
	   );
	}
	
	return Super::OnExecute_Implementation(Target, Parameters);
}

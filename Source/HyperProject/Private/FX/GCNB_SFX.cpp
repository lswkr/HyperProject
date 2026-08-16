// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/GCNB_SFX.h"

#include "Kismet/GameplayStatics.h"


bool UGCNB_SFX::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	UGameplayStatics::PlaySoundAtLocation(
		Target,
		SoundEffect,
		Parameters.Location
	);
	return Super::OnExecute_Implementation(Target, Parameters);
}

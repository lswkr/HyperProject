// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPWeaponBase.h"

FTransform AHPWeaponBase::GetMuzzleSocketTransform() const
{
	FTransform MuzzleSocket = WeaponMesh->GetSocketTransform("MuzzleSocket");

	if (MuzzleSocket.IsValid())
	{
		return MuzzleSocket;	
	}
	return FTransform();
}

UNiagaraSystem* AHPWeaponBase::GetWeaponNiagaraSystem_Implementation() const
{
	if (WeaponInfo.WeaponVFX)
		return WeaponInfo.WeaponVFX;
	return nullptr;
}

FVector AHPWeaponBase::GetWeaponMuzzleLocation_Implementation() const
{
	FVector MuzzleSocketLocation = WeaponMesh->GetSocketTransform("MuzzleSocket").GetLocation();

	return MuzzleSocketLocation;
}

AHPWeaponBase::AHPWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMesh");
	SetRootComponent(WeaponMesh);
}




void AHPWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}


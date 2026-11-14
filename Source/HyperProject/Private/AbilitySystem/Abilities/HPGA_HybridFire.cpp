// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_HybridFire.h"

#include "AbilitySystem/Abilities/HPGA_Fire_Projectile.h"


UHPGA_HybridFire::UHPGA_HybridFire()
{
	FireType = EFireType::BothType;
}

void UHPGA_HybridFire::FireOneShot()
{
	if (IsAiming())
	{
		Fire_HitScan();
	}
	else
	{
		Fire_Projectile();
	}
}

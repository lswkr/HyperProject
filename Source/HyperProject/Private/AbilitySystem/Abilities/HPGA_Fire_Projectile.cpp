// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_Fire_Projectile.h"


UHPGA_Fire_Projectile::UHPGA_Fire_Projectile()
{
	FireType = EFireType::Projectile;
}

void UHPGA_Fire_Projectile::FireOneShot()
{
	Fire_Projectile();
}


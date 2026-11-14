// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_Fire.h"

UHPGA_Fire::UHPGA_Fire()
{
	FireType = EFireType::HitScan;
}

void UHPGA_Fire::FireOneShot()
{
	Fire_HitScan();
}
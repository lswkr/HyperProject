// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPLinearProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"

// AHPLinearProjectile::AHPLinearProjectile()
// {
// 	// ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
// 	//
// 	// ProjectileMovement->bRotationFollowsVelocity = true;
// 	// ProjectileMovement->SetIsReplicated(true);
// 	// ProjectileMovement->InitialSpeed = InitialSpeed;
// 	// ProjectileMovement->MaxSpeed = InitialSpeed;
// 	//
// 	// ProjectileMovement->ProjectileGravityScale = 0.f;
// }

// #if WITH_EDITOR
// void AHPLinearProjectile::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
// {
// 	Super::PostEditChangeProperty(PropertyChangedEvent);
//
// 	FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
//
// 	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHPLinearProjectile, InitialSpeed))
// 	{
// 		if (ProjectileMovement)
// 		{
// 			ProjectileMovement->InitialSpeed = InitialSpeed;
// 			ProjectileMovement->MaxSpeed = InitialSpeed;
// 		}
// 	}
//
// 	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHPLinearProjectile, GravityScale))
// 	{
// 		if (ProjectileMovement)
// 		{
// 			ProjectileMovement->ProjectileGravityScale = GravityScale;
// 		}
// 	}
// }
// #endif

// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPVisualProjectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AHPVisualProjectile::AHPVisualProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	SetRootComponent(BoxComponent);
	
	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	BulletMesh->SetupAttachment(GetRootComponent());
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
}

void AHPVisualProjectile::BeginPlay()
{
	Super::BeginPlay();	
}

#if WITH_EDITOR
void AHPVisualProjectile::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHPVisualProjectile, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHPVisualProjectile, GravityScale))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->ProjectileGravityScale = GravityScale;
		}
	}
}
#endif

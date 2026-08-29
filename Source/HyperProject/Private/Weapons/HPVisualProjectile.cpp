// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPVisualProjectile.h"

#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void AHPVisualProjectile::OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(LocalParticleEffect)
	{
	UGameplayStatics::SpawnEmitterAtLocation(
    					this,
    					LocalParticleEffect,
    					Hit.ImpactPoint,
    			Hit.ImpactNormal.Rotation());
	}
	
				
}

AHPVisualProjectile::AHPVisualProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComponent->OnComponentHit.AddDynamic(this, &AHPVisualProjectile::OnBoxHit);
	SetRootComponent(BoxComponent);
	
	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	BulletMesh->SetupAttachment(GetRootComponent());
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	
}

void AHPVisualProjectile::SetPlayerCharactersToIgnore(TArray<AHPPlayerCharacter*> PlayerCharactersToIgnore)
{
	for (AHPPlayerCharacter* PlayerCharacter : PlayerCharactersToIgnore)
	{
		if (!PlayerCharacter) continue;
		BoxComponent->IgnoreActorWhenMoving(PlayerCharacter,true);	
	}
}

void AHPVisualProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (AHPPlayerCharacter* HPPlayerCharacter = Cast<AHPPlayerCharacter>(GetInstigator()))
	{
		if (HPPlayerCharacter->IsLocallyControlled())
		{
			BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			BulletMesh->SetVisibility(false);
		}
	}
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



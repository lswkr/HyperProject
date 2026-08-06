// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPProjectileBase.h"

#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/LagCompensationComponent.h"
#include "Controller/HPPlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"

AHPProjectileBase::AHPProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	//bReplicates는 ServerSideRewind용과 일반용이 다르므로 BP에서 따로 설정->생성자에서 제외함 

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	SetRootComponent(BoxComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->SetIsReplicated(true);
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = InitialSpeed;
	
	ProjectileMovement->ProjectileGravityScale = 0.f;

}

void AHPProjectileBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

void AHPProjectileBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHPProjectileBase, TeamID)
}

void AHPProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		BoxComponent->OnComponentHit.AddDynamic(this, &AHPProjectileBase::OnBoxComponentHit);
	}
	
}

void AHPProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHPProjectileBase::SetProjectileEffectParams(const FProjectileParams& InProjectileParams)
{
	ProjectileParams = InProjectileParams;
}

void AHPProjectileBase::BindExplosionCallbackFunction(AActor* InPlayerCharacter)
{
	AHPPlayerCharacter* PlayerCharacter = Cast<AHPPlayerCharacter>(InPlayerCharacter);
	if (PlayerCharacter->CanSetMine())
	{
		OnMineExplodeDelegate.BindDynamic(PlayerCharacter, &AHPPlayerCharacter::BombExplosionCallbackFunc);
	}
}

void AHPProjectileBase::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

#if WITH_EDITOR
void AHPProjectileBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
		FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AHPProjectileBase, InitialSpeed))
		{
			if (ProjectileMovement)
			{
				ProjectileMovement->InitialSpeed = InitialSpeed;
				ProjectileMovement->MaxSpeed = InitialSpeed;
			}
		}
	
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AHPProjectileBase, GravityScale))
		{
			if (ProjectileMovement)
			{
				ProjectileMovement->ProjectileGravityScale = GravityScale;
			}
		}

	
}
#endif


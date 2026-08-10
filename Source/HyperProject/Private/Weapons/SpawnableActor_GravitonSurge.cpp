// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/SpawnableActor_GravitonSurge.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"


ASpawnableActor_GravitonSurge::ASpawnableActor_GravitonSurge()
{
	PrimaryActorTick.bCanEverTick = true;

	GravitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("GravitySphere"));
	GravitySphere->OnComponentBeginOverlap.AddDynamic(this, &ASpawnableActor_GravitonSurge::OnSphereBeginOverlap);
	GravitySphere->OnComponentEndOverlap.AddDynamic(this, &ASpawnableActor_GravitonSurge::OnSphereEndOverlap);

	SetRootComponent(GravitySphere);

	SurgeNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("SurgeNiagaraComponent");
	SurgeNiagaraComponent->SetupAttachment(GetRootComponent());
}

void ASpawnableActor_GravitonSurge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		for (AActor* TargetActor : OverlappedActors)
		{
			FVector NewLocation = FMath::VInterpTo(
				TargetActor->GetActorLocation(),
				GetActorLocation(),
				DeltaTime,
				SurgeSpeed
			);
			TargetActor->SetActorLocation(NewLocation);
			
		}
	}
}


void ASpawnableActor_GravitonSurge::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
		SurgeDurationTimerHandle,
		this,
		&ASpawnableActor_GravitonSurge::OnTimerExpired,
		SurgeDuration,
		false);
	}

	SurgeNiagaraComponent->Activate();
}
void ASpawnableActor_GravitonSurge::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryAddTarget(OtherActor);
}

void ASpawnableActor_GravitonSurge::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	RemoveTarget(OtherActor);
}

void ASpawnableActor_GravitonSurge::TryAddTarget(AActor* OtherActor)
{
	if (!OtherActor || OverlappedActors.Contains(OtherActor))
		return;

	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile)
		return;
	
	OverlappedActors.Add(OtherActor);
}

void ASpawnableActor_GravitonSurge::RemoveTarget(AActor* OtherActor)
{
	if (!OtherActor)
	{
		return;
	}

	if (OverlappedActors.Contains(OtherActor))
	{
		OverlappedActors.Remove(OtherActor);
	}
}

void ASpawnableActor_GravitonSurge::OnTimerExpired()
{
	SurgeNiagaraComponent->Deactivate();
	Destroy();
}


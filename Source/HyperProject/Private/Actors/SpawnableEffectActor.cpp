// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SpawnableEffectActor.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"


ASpawnableEffectActor::ASpawnableEffectActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this,&ASpawnableEffectActor::OnBoxBeginOverlap);
	SetRootComponent(BoxComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(GetRootComponent());
}

void ASpawnableEffectActor::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
}

void ASpawnableEffectActor::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Destroy();
}

void ASpawnableEffectActor::Destroyed()
{
	Super::Destroyed();
}

void ASpawnableEffectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveLocationSin(DeltaTime);
	Rotate(DeltaTime);

}

void ASpawnableEffectActor::MoveLocationSin(float DeltaTime)
{
	FVector NewLocation = StartLocation;

	NewLocation.Z += FMath::Sin(GetWorld()->GetTimeSeconds()*MoveSpeed)*WaveHeight;

	SetActorLocation(NewLocation);
}

void ASpawnableEffectActor::Rotate(float DeltaTime)
{
	AddActorLocalRotation(FRotator(0.f, DeltaTime* RotationSpeed,0.f));
}

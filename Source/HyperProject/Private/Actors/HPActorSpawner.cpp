// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/HPActorSpawner.h"

#include "Actors/SpawnableEffectActor.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/HealthPackTimerWidget.h"
#include "Camera/PlayerCameraManager.h"

AHPActorSpawner::AHPActorSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);
	
	SpawnSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnSceneComponent"));
	SpawnSceneComponent->SetupAttachment(GetRootComponent());

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	
	GaugeWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("GaugeWidgetComponent"));
	GaugeWidgetComponent->SetupAttachment(GetRootComponent());

	GaugeWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
}

void AHPActorSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bTurnOn)
	{
		if (!HasAuthority())
		{
			GaugeTime+=DeltaTime;
			GetHealthPackTimerWidget()->OnUpdatePercent(GaugeTime/SpawnTime);

			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				FVector CameraLocation = FVector::ZeroVector;
				FRotator CameraRotation = FRotator::ZeroRotator;
				
				PC->GetPlayerViewPoint(CameraLocation,CameraRotation);
			
				FVector Direction = CameraLocation-GaugeWidgetComponent->GetComponentLocation();
				GaugeWidgetComponent->SetWorldRotation(Direction.Rotation());
			}
		}
	}
}

void AHPActorSpawner::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHPActorSpawner, bTurnOn);
}

void AHPActorSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnEffectActor();
	
	GaugeWidgetComponent->SetVisibility(false);
}



UHealthPackTimerWidget* AHPActorSpawner::GetHealthPackTimerWidget()
{
	if (!HealthPackTimerWidget)
	{
		HealthPackTimerWidget = Cast<UHealthPackTimerWidget>(GaugeWidgetComponent->GetUserWidgetObject());
	}
	return HealthPackTimerWidget;
}
void AHPActorSpawner::SpawnEffectActor()
{
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;


	if (!SpawnableEffectActorClass)
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	
	SpawnedEffectActor = World->SpawnActor<ASpawnableEffectActor> (
		SpawnableEffectActorClass,
		SpawnSceneComponent->GetComponentTransform(),
		SpawnParams
		);

	SpawnedEffectActor->OnDestroyed.AddDynamic(this, &AHPActorSpawner::OnSpawnedActorDestroyed);

	bTurnOn = false;
}

void AHPActorSpawner::OnSpawnedActorDestroyed(AActor* DestroyedActor)
{
	UWorld* World = GetWorld();

	if (!World)
		return;

	bTurnOn = true;
	World->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AHPActorSpawner::SpawnEffectActor,
		SpawnTime,
		false
		);
}

void AHPActorSpawner::OnRep_TurnOnWidget()
{
	GaugeWidgetComponent->SetVisibility(bTurnOn);

	if (bTurnOn)
	{
		GaugeTime = 0;
	}
}


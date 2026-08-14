// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HPActorSpawner.generated.h"

class ASpawnableEffectActor;
class UHealthPackTimerWidget;
class UWidgetComponent;
UCLASS()
class HYPERPROJECT_API AHPActorSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AHPActorSpawner();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere)
	USceneComponent* SpawnSceneComponent;
	
	UPROPERTY(EditAnywhere)
    USceneComponent* RootSceneComponent;
    	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ASpawnableEffectActor> SpawnableEffectActorClass;

	UPROPERTY()
	ASpawnableEffectActor* SpawnedEffectActor;

	FTimerHandle SpawnTimerHandle;

	UPROPERTY(EditAnywhere)
	float SpawnTime;
	
	void SpawnEffectActor();

	UFUNCTION()
	void OnSpawnedActorDestroyed(AActor* DestroyedActor);

	UPROPERTY(ReplicatedUsing = OnRep_TurnOnWidget)
	bool bTurnOn = false;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHealthPackTimerWidget> HealthPackTimerWidgetClass;
	
	UPROPERTY()
	UHealthPackTimerWidget* HealthPackTimerWidget;

	UPROPERTY(EditDefaultsOnly)
	UWidgetComponent* GaugeWidgetComponent ;

	UFUNCTION()
	void OnRep_TurnOnWidget();

	UHealthPackTimerWidget* GetHealthPackTimerWidget();
	float GaugeTime = 0.f; 
};

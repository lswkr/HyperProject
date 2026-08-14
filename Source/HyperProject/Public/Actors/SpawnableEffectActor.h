// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnableEffectActor.generated.h"

class UGameplayEffect;
class UBoxComponent;
class UWidgetComponent;
UCLASS()
class HYPERPROJECT_API ASpawnableEffectActor : public AActor
{
	GENERATED_BODY()
	
public:
	ASpawnableEffectActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void Destroyed() override;
	
public:	
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UBoxComponent* BoxComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* MeshComponent;

	void MoveLocationSin(float DeltaTime);
	void Rotate(float DeltaTime);

	FVector StartLocation;

	UPROPERTY(EditAnywhere)
	float MoveSpeed;

	UPROPERTY(EditAnywhere)
	float RotationSpeed;
	
	UPROPERTY(EditAnywhere)
	float WaveHeight;



	
};

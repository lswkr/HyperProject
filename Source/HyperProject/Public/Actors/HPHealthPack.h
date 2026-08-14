// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpawnableEffectActor.h"
#include "GameFramework/Actor.h"
#include "HPHealthPack.generated.h"

UCLASS()
class HYPERPROJECT_API AHPHealthPack : public ASpawnableEffectActor
{
	GENERATED_BODY()
	
public:	
	AHPHealthPack();

protected:
	virtual void BeginPlay() override;

	virtual void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};

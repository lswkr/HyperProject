// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HPVisualProjectile.generated.h"

class UProjectileMovementComponent;
class UBoxComponent;
UCLASS()
class HYPERPROJECT_API AHPVisualProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AHPVisualProjectile();

protected:
	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	UStaticMeshComponent* BulletMesh;
	
private:
	UPROPERTY(EditDefaultsOnly)
	UProjectileMovementComponent* ProjectileMovementComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UBoxComponent* BoxComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float InitialSpeed = 15000.f;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float GravityScale = 0.f;
};

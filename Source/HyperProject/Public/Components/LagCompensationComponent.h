// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AHPPlayerCharacter;
class AHPPlayerController;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector BoxExtent = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.0f;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AHPPlayerCharacter* Character = nullptr;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed = false;

	UPROPERTY()
	bool bHeadShot = false;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AHPPlayerCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<AHPPlayerCharacter*, uint32> BodyShots;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HYPERPROJECT_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AHPPlayerCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	/* Hitscan */
	FServerSideRewindResult ServerSideRewind(AHPPlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	/* Projectile */
	FServerSideRewindResult ProjectileServerSideRewind(
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		const FProjectileApplyEffectParams& ProjectileParams,
		float HitTime
	);
	
	/* Explosion Projectile Begin */
	FServerSideRewindResult ExplosionServerSideRewind(AHPPlayerCharacter* HitCharacter,
	                                                  const FVector_NetQuantize& OriginLocation, float InnerRadius,
	                                                  float OuterRadius, float HitTime, FVector_NetQuantize& HitLocation);
	UFUNCTION(Server, Reliable)
	void ExplosionServerApplyValidHit_HitObject(
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime,
		const FProjectileApplyEffectParams& ProjectileApplyEffectParams,
		const TArray<AHPPlayerCharacter*>& OverlappedCharacters
	);

	UFUNCTION(Server, Reliable)
	void ExplosionServerApplyValidHit_HitCharacter(
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime,
		const FProjectileApplyEffectParams& ProjectileApplyEffectParams,
		const TArray<AHPPlayerCharacter*>& OverlappedCharacters
	);
	
	/* Explosion Projectile End */

	/* Point Projectile Begin */
	UFUNCTION(Server, Reliable)
	void ProjectileServerApplyValidHit(
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		const FProjectileApplyEffectParams& ProjectileApplyEffectParams,
		float HitTime
	);
	/* Point Projectile End */

	/*  Spawning Projectile Begin */
	UFUNCTION(Server, Reliable)
	void SpawningProjectileServerApplyValidHit_HitCharacter(
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime,
		const FProjectileApplyEffectParams& ProjectileApplyEffectParams
		);

	UFUNCTION(Server, Reliable)
	void SpawningProjectileServerApplyValidHit_HitObject(
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		//float HitTime,
		const FProjectileApplyEffectParams& ProjectileApplyEffectParams
	);
	/*  Spawning Projectile End */
	
protected:
	virtual void BeginPlay() override;
	void CaptureFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void CacheBoxPositions(AHPPlayerCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AHPPlayerCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AHPPlayerCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AHPPlayerCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(AHPPlayerCharacter* HitCharacter, float HitTime);

	/** 
	* Hitscan
	*/
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation
		);

	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		const FProjectileApplyEffectParams& ProjectileParams,
		float HitTime
	);

	FServerSideRewindResult ProjectileConfirmHit_ForObject(
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity
	);

	FServerSideRewindResult ExplosionConfirmHit(const FFramePackage& Package,
	AHPPlayerCharacter* HitCharacter, const FVector_NetQuantize& OriginLocation, float InnerRadius, float OuterRadius, float HitTime, FVector_NetQuantize& HitLocation);

private:

	UPROPERTY()
	AHPPlayerCharacter* HPCharacter;

	UPROPERTY()
	AHPPlayerController* PlayerController;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag HitVFXCueTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag HitSoundCueTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag LocalBodyHitSoundCueTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag LocalHeadHitSoundCueTag;
};

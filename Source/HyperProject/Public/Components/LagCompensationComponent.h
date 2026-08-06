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
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AHPPlayerCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
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

	// UFUNCTION(Server, Reliable)
	// FServerSideRewindResult ServerCheckValidHit(AHPPlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	/** 
	* Hitscan
	*/
	FServerSideRewindResult ServerSideRewind(AHPPlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileServerSideRewind(
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	// /** 
	// * Shotgun
	// */
	// FShotgunServerSideRewindResult ShotgunServerSideRewind(
	// 	const TArray<AHPPlayerCharacter*>& HitCharacters,
	// 	const FVector_NetQuantize& TraceStart,
	// 	const TArray<FVector_NetQuantize>& HitLocations,
	// 	float HitTime);
	//
	// UFUNCTION(Server, Reliable)
	// void ServerScoreRequest(
	// 	AHPPlayerCharacter* HitCharacter,
	// 	const FVector_NetQuantize& TraceStart,
	// 	const FVector_NetQuantize& HitLocation,
	// 	float HitTime
	// );
	//
	UFUNCTION(Server, Reliable)
	void ProjectileServerApplyValidHit(
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime,
		const FProjectileApplyEffectParams& ProjectileApplyEffectParams
	);
	
	//
	// UFUNCTION(Server, Reliable)
	// void ShotgunServerScoreRequest(
	// 	const TArray<AHPPlayerCharacter*>& HitCharacters,
	// 	const FVector_NetQuantize& TraceStart,
	// 	const TArray<FVector_NetQuantize>& HitLocations,
	// 	float HitTime
	// );

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
		const FVector_NetQuantize& HitLocation);

	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AHPPlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/** 
	* Shotgun
	*/

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);

private:

	UPROPERTY()
	AHPPlayerCharacter* HPCharacter;

	UPROPERTY()
	AHPPlayerController* PlayerController;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};

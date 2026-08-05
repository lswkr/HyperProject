// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HPCombatComponent.generated.h"

class UBoxComponent;
class UHPAbilitySystemComponent;
class AHPWeaponBase;
class AHPPlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HYPERPROJECT_API UHPCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHPCombatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE FVector GetHitTargetImpactPoint() const { return HitTargetImpactPoint; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AHPWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; } 
	
protected:
	
	virtual void BeginPlay() override;

	void AttachActorToCharacterMeshSocket(AActor* ActorToAttach, FName SocketName);

	UFUNCTION()
	void OnRep_EquipWeapon();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TSubclassOf<AHPWeaponBase> GetWeaponClass(int32 WeaponIndex) const;

	void EquipWeapon(AHPWeaponBase* NewWeapon);

	UFUNCTION(BlueprintCallable)
	void AddPendingBullets(int32 AddedBullets);
	
	int32 GetClientPendingBullets(int32 NewServerBullets);
	void SetServerBullets(int32 NewBullets);

	UFUNCTION(BlueprintCallable)
	void ReturnHitTargetFromMuzzleSocket(FVector& StartLocation, FHitResult& OutHitTarget);

	UFUNCTION(BlueprintCallable)
	FVector GetHitImpactPoint();

	UFUNCTION(BlueprintCallable)
	FVector GetMuzzleSocketLocation();
	
	void CaptureAimStartTime();
	void CaptureAimEndTime();
	void ClearAimTimes();
	float GetDurationBetweenAim();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ToggleMeleeHitBox(bool TurnOn);

	FORCEINLINE void SetAiming(bool TurnOn) { bIsAiming=TurnOn; }
	
private:
	UPROPERTY(EditDefaultsOnly)
	TMap<int32, TSubclassOf<AHPWeaponBase>> Weapons;

	int CurrentWeaponIndex = 0;

	UPROPERTY()
	AHPPlayerCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquipWeapon)
	AHPWeaponBase* CurrentWeapon;

	UPROPERTY()
	UHPAbilitySystemComponent* HPASC;

	UPROPERTY(EditDefaultsOnly)
	FName LeftHandSocketName;

	UPROPERTY(EditDefaultsOnly)
	FName RightHandSocketName;

	FVector HitTargetImpactPoint;

	void TraceUnderCrossHair(FHitResult& TraceHitResult) const;

	int32 ClientPendingBullets;
	int32 LastServerBullets;
	int32 CurrentServerBullets;

	/* Aiming Start */
	float AimStartTime;
	float AimEndTime;

	bool bIsAiming;
	void InterpFOV(float DeltaTime);

	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;
	/* Aiming End */

	/* Server Side Rewind Begin*/
public:
	void ShouldUseServerSideRewind(bool TooHighPing);
	FORCEINLINE bool IsUsingServerSideRewind() const { return bUseServerSideRewind; };
private:
	UPROPERTY(Replicated)
	bool bUseServerSideRewind = false;
	/* Server Side Rewind End*/
};

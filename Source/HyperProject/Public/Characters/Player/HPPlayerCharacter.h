// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "Characters/HPCharacterBase.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "HPGenericTypes.h"
#include "Interfaces/CombatInterface.h"
#include "UI/WidgetController/HPWidgetController.h"
#include "HPPlayerCharacter.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusTagChangedDelegate, bool, TurnOn);


class UCameraComponent;
class USpringArmComponent;
class UHPCombatComponent;
class UBoxComponent;
class UDamageContributionComponent;
class UInputAction;
class UInputMappingContext;
class AHPWeaponBase;
class UWidgetComponent;
class ULagCompensationComponent;

struct FInputActionValue;



UCLASS()
class HYPERPROJECT_API AHPPlayerCharacter : public AHPCharacterBase,public ICombatInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AHPPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void PawnClientRestart() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void ServerSideInit() override;
	
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE bool GetIsCrouching() const { return bIsCrouching; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE ETurningInPlace GetTurningInPlaceState() const { return TurningInPlace;};
	AHPWeaponBase* GetCurrentWeapon() const;

	FVector GetHitTargetImpactPoint() const;

	ECombatState GetCombatState() const;
	
	virtual void ClientSideInit() override; //InitOverlay용

	void HandleRespawn();
	void HandleDeath();
	void Death();
	
	virtual void ToggleMeleeHitBox_Implementation(bool TurnOn) override;

	/* CombatInterface Begin */
	virtual bool IsUsingServerRewind_Implementation() const override;
	/* CombatInterface Begin */

	/* IGenericTeamAgentInterface Begin */
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	/* IGenericTeamAgentInterface End */

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE bool CanSetMine() const { return bCanSetMine; }

	UFUNCTION()//서버서 호출되므로 위젯 바꾸기 위해선 클라에 전달해하는 rpc같은 것 필요
	void BombExplosionCallbackFunc();

	FORCEINLINE UCameraComponent* GetPlayerCamera() const { return Camera; }

	/* HitConfirm Effect Begin*/
public:
	UFUNCTION(Client,Unreliable)
	void Client_HitConfirm(bool bIsHeadShot);

	UPROPERTY(EditDefaultsOnly , Category = "HitConfirm")
	USoundBase* BodyHitSound;
	
	UPROPERTY(EditDefaultsOnly , Category = "HitConfirm")
	USoundBase* HeadHitSound;
	
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	/* Death And Respawn Begin */
	UFUNCTION()
	void DeathTagUpdated(FGameplayTag GameplayTag, int TagCount);

	UPROPERTY(EditDefaultsOnly,Category = "Death")
	UAnimMontage* DeathAnimMontage;

	void PlayDeadAnimation();
	void DeathMontageFinished();

	FTimerHandle DeathMontageTimerHandle;

	bool IsDead() const;
	/* Death And Respawn End */
	
	/* Widget Begin*/
	void ShowOverHeadWidget();

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	UWidgetComponent* OverHeadWidgetComponent;

	FTimerHandle OverHeadWidgetTimerHandle;

	void UpdateOverHeadWidgetVisibility();
	void ConfigureOverHeadWidget();
	void OnHealBanTagChanged(FGameplayTag Tag, int32 NewCount);
	void BindCallbacksToDependencies();
	void BroadcastInitialValues();

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	float OverHeadWidgetVisibilityPeriod = 1.f;
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	float OverHeadWidgetVisibilityRangeSquared = 10000000.f;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Status")
	FOnStatusTagChangedDelegate OnHealBanTagChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnHealthChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnMaxHealthChangedDelegate;
	/* Widget End*/
	
	/* Melee Hit Begin */
	UPROPERTY()
	TSet<AActor*> OverlappedActors;

	UPROPERTY(EditDefaultsOnly,Category = "MeleeHit")
	UBoxComponent* MeleeHitBoxComponent;

	UPROPERTY(EditDefaultsOnly,Category = "MeleeHit")
	
	FName MeleeHitBoxSocketName;


	
	UPROPERTY(EditDefaultsOnly,Category = "MeleeHit")
	UAnimMontage* MeleeHitAnimMontage;
	virtual UAnimMontage* GetMeleeHitAnimMontage_Implementation() const override;
	virtual void ClearMeleeHitSet_Implementation() override;
	/* Melee Hit End */

	/* Reload Begin */
	virtual UAnimMontage* GetReloadAnimMontage_Implementation() const override;
	UPROPERTY(EditDefaultsOnly,Category = "Reload")
	UAnimMontage* ReloadAnimMontage;
	/* Reload End */
	virtual FVector GetUltMuzzleSocketLocation_Implementation() const override;
	virtual FVector GetWeaponSocketLocation_Implementation() const override;
	virtual FVector GetThrowingHandSocketLocation_Implementation() const override;
	virtual FVector GetHitImpactPoint_Implementation() const override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Combat")
	UDamageContributionComponent* DamageContributionComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<FName> DesignatedBoneNamesToHide;

	
	/**Input Start**/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* CharacterMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<EHPAbilityInputID, UInputAction*>	GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<int32, UInputAction*>	WeaponSelectInputActions;

	
	void HandleMove(const FInputActionValue& InputActionValue);
	void HandleLook(const FInputActionValue& InputActionValue);
	void HandleCrouch(const FInputActionValue& InputActionValue);
	void HandleAbilityInputPressed(const FInputActionValue& InputActionValue, EHPAbilityInputID InputID);
	void HandleAbilityInputReleased(const FInputActionValue& InputActionValue, EHPAbilityInputID InputID);
	void HandleWeaponChange(const FInputActionValue& InputActionValue, int32 WeaponIndex);

	UFUNCTION()
	void OnBoxBeginOverlap (UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	/**Input End**/
	
	void SpawnWeapon(int32 WeaponIndex);
	
	float CalculateXYSpeed() const;
	void CalculateAO_Pitch();
	
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaSeconds);
	void RotateInPlace(float DeltaSeconds);
	void SimulatedProxiesTurn();
	
	void HideDesignatedBones();
	
	
	
	/*Rotation and TurnInPlace*/
	bool bIsCrouching;
	void AimOffset(float DeltaSeconds);
	float AO_Yaw;
	float AO_Pitch;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;
	float TimeSinceLastMovementReplication;
	bool bRotateRootBone;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TurnThreshold = .5f;

	bool bIsAiming = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	bool bCanSetMine = false;

	UPROPERTY(EditAnywhere, Replicated)
	FGenericTeamId TeamID;

	/* Server Side Rewind Begin*/
public:
	friend class ULagCompensationComponent;
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; }
	
protected:
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;
	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensationComponent;
	
	/* Server Side Rewind End*/

public:
	AHPPlayerController* GetHPPlayerController();
private:
	UPROPERTY()
	AHPPlayerController* HPPlayerController;
};

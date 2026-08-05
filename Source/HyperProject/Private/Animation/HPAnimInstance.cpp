// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/HPAnimInstance.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "HPGameplayTags.h"
#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/HPWeaponBase.h"

void UHPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	HPPlayerCharacter = Cast<AHPPlayerCharacter> (TryGetPawnOwner());

	if (HPPlayerCharacter)
	{
		ASC = Cast<UHPAbilitySystemComponent>(HPPlayerCharacter->GetAbilitySystemComponent());
	}
}

void UHPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (HPPlayerCharacter == nullptr)
	{
		HPPlayerCharacter = Cast<AHPPlayerCharacter> (TryGetPawnOwner());
	}
	if (HPPlayerCharacter == nullptr)
	{
		return;
	}


	FVector Velocity = HPPlayerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = HPPlayerCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = HPPlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bIsCrouched = HPPlayerCharacter->bIsCrouched;
	bAiming = ASC && ASC->HasMatchingGameplayTag(FHPGameplayTags::Get().State_Combat_Aiming);
	TurningInPlace = HPPlayerCharacter->GetTurningInPlaceState();
	bRotateRootBone = HPPlayerCharacter->ShouldRotateRootBone();
	
	//Yaw Offset
	FRotator AimRotation = HPPlayerCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(HPPlayerCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = HPPlayerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = HPPlayerCharacter->GetAO_Yaw();
	AO_Pitch = HPPlayerCharacter->GetAO_Pitch();

	EquippedWeapon = HPPlayerCharacter->GetCurrentWeapon();
	
	if (EquippedWeapon && EquippedWeapon->GetWeaponMesh() && HPPlayerCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		HPPlayerCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);

		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (HPPlayerCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("RightHandSocket"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - HPPlayerCharacter->GetHitTargetImpactPoint()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.f);
		}
	}
	
	// bUseFABRIK = HPPlayerCharacter->GetCombatState() == ECombatState::Unoccupied;
	// bool bFABRIKOverride = HPPlayerCharacter->IsLocallyControlled() && HPPlayerCharacter->bFinishedSwapping;
	//
	// if (bFABRIKOverride)
	// {
	// 	bUseFABRIK = !HPPlayerCharacter->IsLocallyReloading();
	// }
	// bUseAimOffsets = HPPlayerCharacter->GetCombatState() == ECombatState::Unoccupied /*&& !HPPlayerCharacter->GetDisableGameplay()*/;
	// bTransformRightHand = HPPlayerCharacter->GetCombatState() == ECombatState::Unoccupied /* && !HPPlayerCharacter->GetDisableGameplay()*/;
}



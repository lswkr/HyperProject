// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HPCombatComponent.h"

#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/HPCharacterBase.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/HPWeaponBase.h"

UHPCombatComponent::UHPCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	//처음 Weapon 생성 시 0번 인덱스와 다르게 하여 Weapon이 생성 로직의 if문을 통과하기 위해 CurrentWeaponIndex를 -1로 초기화
	CurrentWeaponIndex=-1;
}

void UHPCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHPCombatComponent, CurrentWeapon);
}


void UHPCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<AHPPlayerCharacter>(GetOwner());

	if (Character)
	{
		HPASC = Cast<UHPAbilitySystemComponent>(Character->GetAbilitySystemComponent());

		DefaultFOV = Character->GetPlayerCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;
	}
}


void UHPCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrossHair(HitResult);
		HitTargetImpactPoint = HitResult.ImpactPoint;

		InterpFOV(DeltaTime);
	}
}

TSubclassOf<AHPWeaponBase> UHPCombatComponent::GetWeaponClass(int32 WeaponIndex) const
{
	if (WeaponIndex == CurrentWeaponIndex) return nullptr;

	if (!Weapons.Contains(WeaponIndex))
		return nullptr;
	
	return Weapons[WeaponIndex];
}


void UHPCombatComponent::EquipWeapon(AHPWeaponBase* NewWeapon)
{
	CurrentWeapon = NewWeapon;
	AttachActorToCharacterMeshSocket(NewWeapon, RightHandSocketName);
}

// void UHPCombatComponent::AddPendingBullets(int32 AddedBullets)
// {
// 	ClientPendingBullets += AddedBullets;
// }

// int32 UHPCombatComponent::GetClientPendingBullets(int32 NewServerBullets)
// {
// 	int32 DifferenceBetweenDelegate = NewServerBullets - LastServerBullets;
// 	ClientPendingBullets -= DifferenceBetweenDelegate;
//
// 	return ClientPendingBullets;
// }
//
// void UHPCombatComponent::SetServerBullets(int32 NewBullets)
// {
// 	LastServerBullets = NewBullets;
// }

//Deprecated Function
void UHPCombatComponent::ReturnHitTargetFromMuzzleSocket(FVector& StartLocation, FHitResult& OutHitTarget)
{
	UWorld* World = GetWorld();

	if (!World)
		return;
	FTransform TraceStart = CurrentWeapon->GetMuzzleSocketTransform();

	if (!TraceStart.IsValid())
		return;
	StartLocation = TraceStart.GetLocation();

	FVector TraceDirection = (HitTargetImpactPoint-StartLocation).GetSafeNormal();
	FVector End = HitTargetImpactPoint + TraceDirection*100.f;
//	FVector End = TraceStart.GetLocation() + (HitTargetImpactPoint - TraceStart.GetLocation());

	World->LineTraceSingleByChannel(
		OutHitTarget,
		TraceStart.GetLocation(),
		End,
		ECollisionChannel::ECC_Visibility
	);
	//DrawDebugLine(GetWorld(),StartLocation, End,FColor::Yellow, true, 5, 10);
	
	if (HitTargetImpactPoint == FVector::ZeroVector) //맞은 것이 없다면 임의의 길이로 EndPoint지정
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent NoTargetHit"));
		FVector2D ViewportSize;

		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}
	
		FVector2D CrossHairLocation = FVector2D(ViewportSize.X/2.f, ViewportSize.Y/2.f);
		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;

		//NEXTTHINGTODO: 플레이어에서 직접 컨트롤러 가져와 해보기
		bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this,0),
			CrossHairLocation,
			CrosshairWorldPosition,
			CrosshairWorldDirection
			);

		if (bScreenToWorld)
		{
			//라인트레이스 시작점 캐릭터 몸 앞으로 당겨오기
			FVector StartPoint = CrosshairWorldPosition;

			if (Character)
			{
				float DistanceToCharacter = (Character->GetActorLocation() - StartPoint).Size();
				StartPoint += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
			}

			FVector BeamEndPoint = StartPoint + CrosshairWorldDirection * 1000.f;
			OutHitTarget.ImpactPoint = BeamEndPoint;
		}
		OutHitTarget.bBlockingHit = false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent TargetHit! %s"),*End.ToString());
	}
}

FVector UHPCombatComponent::GetHitImpactPoint()
{
	return HitTargetImpactPoint;
}

FVector UHPCombatComponent::GetMuzzleSocketLocation()
{
	return CurrentWeapon->GetMuzzleSocketTransform().GetLocation();
}



void UHPCombatComponent::CaptureAimStartTime()
{
	UE_LOG(LogTemp,Warning,TEXT("UHPCombatComponent::CaptureAimStartTime"));
	AimStartTime = GetWorld()->GetTimeSeconds();
}

void UHPCombatComponent::CaptureAimEndTime()
{
	UE_LOG(LogTemp,Warning,TEXT("UHPCombatComponent::CaptureAimEndTime"));
	AimEndTime = GetWorld()->GetTimeSeconds();
}

void UHPCombatComponent::ClearAimTimes()
{
	UE_LOG(LogTemp,Warning,TEXT("UHPCombatComponent::ClearAimTimes"));
	AimStartTime = 0.f;
	AimEndTime = 0.f;
}

float UHPCombatComponent::GetDurationBetweenAim()
{
	return AimEndTime - AimStartTime;
}


void UHPCombatComponent::TraceUnderCrossHair(FHitResult& TraceHitResult) const
{
	FVector2D ViewportSize;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	
	FVector2D CrossHairLocation = FVector2D(ViewportSize.X/2.f, ViewportSize.Y/2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//NEXTTHINGTODO: 플레이어에서 직접 컨트롤러 가져와 해보기
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		CrossHairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
		);

	if (bScreenToWorld)
	{
		//라인트레이스 시작점 캐릭터 몸 앞으로 당겨오기
		FVector StartPoint = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - StartPoint).Size();
			StartPoint += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector EndPoint = StartPoint + CrosshairWorldDirection * 100000.f;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			StartPoint,
			EndPoint,
			ECollisionChannel::ECC_Visibility
			);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = EndPoint;
		}
	}
}

void UHPCombatComponent::InterpFOV(float DeltaTime)
{
	if (CurrentWeapon==nullptr) return;

	if (!CurrentWeapon->CanAim()) return; //조준 불가한 무기일 경우 fast return
	
	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, CurrentWeapon->GetZoomedFOV(), DeltaTime, CurrentWeapon->GetZoomInterpSpeed());	
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, CurrentWeapon->GetZoomInterpSpeed());	
	}

	if (Character && Character->GetPlayerCamera())
	{
		Character->GetPlayerCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UHPCombatComponent::ShouldUseServerSideRewind(bool TooHighPing)
{
	bUseServerSideRewind = !TooHighPing;
}

// void UHPCombatComponent::TryDestroyBoundedSpawnedActor()
// {
// 	if (GetOwner()->HasAuthority())
// 	{
// 		if (SpawnedActorBySpawningProjectile)
// 		{
// 			SpawnedActorBySpawningProjectile->Destroy();
// 		}
// 	}
// }


void UHPCombatComponent::ToggleMeleeHitBox(bool TurnOn)
{
	if (GetOwner()->Implements<UCombatInterface>())
	{
		ICombatInterface::Execute_ToggleMeleeHitBox(GetOwner(), TurnOn);
	}
}

void UHPCombatComponent::AttachActorToCharacterMeshSocket(AActor* ActorToAttach, FName SocketName)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("Attach Actor to CharacterMeshSocket"));
	FAttachmentTransformRules AttachmentRules(
		EAttachmentRule::SnapToTarget, 
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,  
		true
		);
	ActorToAttach->AttachToComponent(Character->GetMesh(), AttachmentRules, SocketName);
	
}

void UHPCombatComponent::OnRep_EquipWeapon()
{
	AttachActorToCharacterMeshSocket(CurrentWeapon, "RightHandSocket");
}


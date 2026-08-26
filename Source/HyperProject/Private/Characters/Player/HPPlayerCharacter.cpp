// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/HPPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HPGameplayTags.h"
#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "Components/BoxComponent.h"
#include "Components/DamageContributionComponent.h"
#include "Components/HPCombatComponent.h"
#include "Controller/HPPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actors/HPControlPoint.h"
#include "Actors/RespawnPlayerStart.h"
#include "Components/LagCompensationComponent.h"
#include "Components/WidgetComponent.h"
#include "GameMode/ControlPointGameMode.h"
#include "HyperProject/HyperProject.h"
#include "Kismet/GameplayStatics.h"

#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "UI/HUD/HPHUD.h"
#include "UI/Widget/HPOverheadWidget.h"
#include "UI/Widget/HPUserWidget.h"
#include "Weapons/HPWeaponBase.h"

AHPPlayerCharacter::AHPPlayerCharacter()
{
	HPAbilitySystemComponent->RegisterGameplayTagEvent(FHPGameplayTags::Get().State_Dead, EGameplayTagEventType::NewOrRemoved).
	AddUObject(this,&AHPPlayerCharacter::DeathTagUpdated);
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	MeleeHitBoxComponent = CreateDefaultSubobject<UBoxComponent>("MeleeHitBoxComponent");
	MeleeHitBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeleeHitBoxComponent->SetupAttachment(GetMesh());
	MeleeHitBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AHPPlayerCharacter::OnBoxBeginOverlap);
	
	// bUseControllerRotationYaw = false;
	// GetCharacterMovement()->bOrientRotationToMovement = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
	
	TurningInPlace = ETurningInPlace::NotTurning;
	
	CombatComponent = CreateDefaultSubobject<UHPCombatComponent>("CombatComponent");

	DamageContributionComponent = CreateDefaultSubobject<UDamageContributionComponent>("DamageContributionComponent");

	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("OverHeadWidgetComponent");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	/* Server Side Rewind*/
	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>("LagCompensationComponent");

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AHPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RotateInPlace(DeltaSeconds);

	ShowOverHeadWidget();
}

void AHPPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	UE_LOG(LogTemp, Warning,TEXT("PawnClientRestart"));
	APlayerController* OwningPlayerController = GetController<APlayerController>();

	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

		if (InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(DefaultMappingContext);
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
			
			if (CharacterMappingContext)
			{
				InputSubsystem->AddMappingContext(CharacterMappingContext, 1);
			}
		}
	}
}

void AHPPlayerCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimulatedProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AHPPlayerCharacter::ServerSideInit()
{
	Super::ServerSideInit();
	DamageContributionComponent->BindAbilitySystemComponent(HPAbilitySystemComponent);
	//첫 생성 시 PlayerStart초기화를 위해
	if (GetController()->StartSpot.IsValid())
	{
		ARespawnPlayerStart* PlayerStart = Cast<ARespawnPlayerStart>(GetController()->StartSpot.Get());
		if (PlayerStart)
		{
			UE_LOG(LogTemp, Warning,TEXT("Init StartSpot"));
			PlayerStart->SetOccupied(false);
		}	
	}
	//BroadcastInitialValues();
}


void AHPPlayerCharacter::Client_HitConfirm_Implementation(bool bIsHeadShot)
{
	UE_LOG(LogTemp, Warning,TEXT("HitConfirm Called"));
	UGameplayStatics::PlaySound2D(this, bIsHeadShot? HeadHitSound :BodyHitSound);

	AHPPlayerController* CurrentHPPlayerController = GetHPPlayerController();

	if (CurrentHPPlayerController)
	{
		CurrentHPPlayerController->PlayHitFeedbackWidget(bIsHeadShot);
	}
}

void AHPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHPPlayerCharacter::HandleMove);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHPPlayerCharacter::HandleLook);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AHPPlayerCharacter::HandleCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AHPPlayerCharacter::HandleCrouch);
	}
	for (const TPair<EHPAbilityInputID, UInputAction*>& InputActionPair : GameplayAbilityInputActions)
	{
		EnhancedInputComponent->BindAction(InputActionPair.Value, ETriggerEvent::Started, this, &AHPPlayerCharacter::HandleAbilityInputPressed, InputActionPair.Key);
		EnhancedInputComponent->BindAction(InputActionPair.Value, ETriggerEvent::Completed, this, &AHPPlayerCharacter::HandleAbilityInputReleased, InputActionPair.Key);
	}
	for (const TPair<int32, UInputAction*>& InputActionPair : WeaponSelectInputActions)
	{
		EnhancedInputComponent->BindAction(InputActionPair.Value, ETriggerEvent::Started, this, &AHPPlayerCharacter::HandleWeaponChange, InputActionPair.Key);
	}

	
}

void AHPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnWeapon(0);
	ConfigureOverHeadWidget();

	CachedMaxWalkSpeed = GetMovementComponent()->GetMaxSpeed();
}

void AHPPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	HideDesignatedBones();

	 if (LagCompensationComponent)
	 {
	 	LagCompensationComponent->HPCharacter = this;
	 	if (Controller)
	 	{
	 		LagCompensationComponent->PlayerController = Cast<AHPPlayerController>(Controller);
	 	}
	 }
}

void AHPPlayerCharacter::BombExplosionCallbackFunc()
{
	if (bCanSetMine)
	{
		//NEXTTHINGTODO 뭔지 확인하기
		UE_LOG(LogTemp, Warning, TEXT("폭탄 스킬 위젯 바꾸기"));
	}
}

#if WITH_EDITOR
void AHPPlayerCharacter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass,MeleeHitBoxSocketName))
	{
		MeleeHitBoxComponent->AttachToComponent(GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,MeleeHitBoxSocketName);
	}
}

#endif


void AHPPlayerCharacter::DeathTagUpdated(FGameplayTag GameplayTag, int TagCount)
{
	UE_LOG(LogTemp, Warning,TEXT("DeathTagUpdated: TagCount = %d"), TagCount);
	if (TagCount==1)
	{
		HandleDeath();
	}
	else if (TagCount==0)
	{
		HandleRespawn();
	}
}

void AHPPlayerCharacter::PlayDeadAnimation()
{
	if (DeathAnimMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathAnimMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &AHPPlayerCharacter::DeathMontageFinished, MontageDuration-0.8f);
	}
}

void AHPPlayerCharacter::DeathMontageFinished()
{
}

bool AHPPlayerCharacter::IsDead() const
{
	if (!HPAbilitySystemComponent)
		return false;
	
	return HPAbilitySystemComponent->HasMatchingGameplayTag(FHPGameplayTags::Get().State_Dead);
}

void AHPPlayerCharacter::ShowOverHeadWidget()
{
	if (HasAuthority())
		return;
	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FVector CameraLocation = FVector::ZeroVector;
		FRotator CameraRotation = FRotator::ZeroRotator;
				
		PC->GetPlayerViewPoint(CameraLocation,CameraRotation);
		
		FVector Direction = CameraLocation - OverHeadWidgetComponent->GetComponentLocation();
		OverHeadWidgetComponent->SetWorldRotation(Direction.Rotation());
	}	
}

UAnimMontage* AHPPlayerCharacter::GetMeleeHitAnimMontage_Implementation() const
{
	return MeleeHitAnimMontage;
}

UAnimMontage* AHPPlayerCharacter::GetReloadAnimMontage_Implementation() const
{
	return ReloadAnimMontage;
}

FVector AHPPlayerCharacter::GetUltMuzzleSocketLocation_Implementation() const
{
	return GetMesh()->GetSocketLocation(TEXT("MuzzleSocket_Ult"));
}

FVector AHPPlayerCharacter::GetWeaponSocketLocation_Implementation() const
{
	return CombatComponent->GetMuzzleSocketLocation();
}

FVector AHPPlayerCharacter::GetThrowingHandSocketLocation_Implementation() const
{
	return GetMesh()->GetSocketLocation(TEXT("ThrowingSocket"));
}

FVector AHPPlayerCharacter::GetHitImpactPoint_Implementation() const
{
	return CombatComponent->GetHitImpactPoint();
}

void AHPPlayerCharacter::ClearMeleeHitSet_Implementation()
{
	if (!OverlappedActors.IsEmpty())
	{
		OverlappedActors.Empty();
	}
}


void AHPPlayerCharacter::HandleMove(const FInputActionValue& InputActionValue)
{
	FVector2D InputValue = InputActionValue.Get<FVector2D>();

	bool bIsAiming = false;
	if (HPAbilitySystemComponent->HasMatchingGameplayTag(FHPGameplayTags::Get().State_Combat_Aiming))
	{
		bIsAiming=true;
	}
	InputValue.Normalize();

	if (InputValue.X != 0)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

		AddMovementInput(Direction, InputValue.X);
	}

	if (InputValue.Y != 0)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, InputValue.Y);
	}
	
	bIsAiming ? GetCharacterMovement()->MaxWalkSpeed*= 1/2 : GetCharacterMovement()->MaxWalkSpeed = CachedMaxWalkSpeed;
}

void AHPPlayerCharacter::HandleLook(const FInputActionValue& InputActionValue)
{
	FVector2D InputValue = InputActionValue.Get<FVector2D>();
	float Multiplier = 1.f;
	if (HPAbilitySystemComponent->HasMatchingGameplayTag(FHPGameplayTags::Get().State_Combat_Aiming))
	{
		Multiplier=0.7f;
	}
	if (InputValue.X != 0)
	{
		AddControllerYawInput(InputValue.X*Multiplier);
	}
	if (InputValue.Y != 0)
	{
		AddControllerPitchInput(-InputValue.Y*Multiplier);
	}
}

void AHPPlayerCharacter::HandleCrouch(const FInputActionValue& InputActionValue)
{
	//NEXTTHINGTODO: 웅크리기 시 점프 불가->필요 시 수정하기
	bool InputValue = InputActionValue.Get<bool>();

	if (InputValue)
	{
		if (!bIsCrouched)
		{	
			Crouch();
		}
	}
	else
	{
		if (bIsCrouched)
		{
			UnCrouch();
		}
	}
	bIsCrouched ? GetCharacterMovement()->MaxWalkSpeed = 1/2 : GetCharacterMovement()->MaxWalkSpeed = CachedMaxWalkSpeed;
}

void AHPPlayerCharacter::HandleAbilityInputPressed(const FInputActionValue& InputActionValue, EHPAbilityInputID InputID)
{
	if (GetAbilitySystemComponent() == nullptr)
	{
		return;
	}
	GetAbilitySystemComponent() -> AbilityLocalInputPressed(static_cast<int32>(InputID));
}

void AHPPlayerCharacter::HandleAbilityInputReleased(const FInputActionValue& InputActionValue, EHPAbilityInputID InputID)
{
	if (GetAbilitySystemComponent() == nullptr)
	{
		return;
	}
	GetAbilitySystemComponent() -> AbilityLocalInputReleased(static_cast<int32>(InputID));
}

void AHPPlayerCharacter::HandleWeaponChange(const FInputActionValue& InputActionValue, int32 WeaponIndex)
{
	SpawnWeapon(WeaponIndex);
}

void AHPPlayerCharacter::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (OtherActor != this && !OverlappedActors.Contains(OtherActor))
		{
			FGameplayEventData EventData;
			EventData.Target = OtherActor;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,FHPGameplayTags::Get().Event_MeleeHit, EventData);
			OverlappedActors.Add(OtherActor);
		}
	}
	else
	{
		//NEXTTHINGTODO: 효과 주기
	}
}

void AHPPlayerCharacter::SpawnWeapon(int32 WeaponIndex)
{
	TSubclassOf<AHPWeaponBase> WeaponClass = CombatComponent->GetWeaponClass(WeaponIndex);

	if (WeaponClass == nullptr)
		return;
	
	UWorld* World = GetWorld();
	
	AHPWeaponBase* ChangedWeapon = World->SpawnActor<AHPWeaponBase>(WeaponClass);

	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(ChangedWeapon);
	}

	if (HPAbilitySystemComponent)
	{
		HPAbilitySystemComponent->OnEquipWeapon(*ChangedWeapon);
	}
}

AHPWeaponBase* AHPPlayerCharacter::GetCurrentWeapon() const
{
	return CombatComponent->GetCurrentWeapon();
}

FVector AHPPlayerCharacter::GetHitTargetImpactPoint() const
{
	return CombatComponent->GetHitTargetImpactPoint();
}

ECombatState AHPPlayerCharacter::GetCombatState() const
{
	return HPAbilitySystemComponent->GetCombatState();
}

void AHPPlayerCharacter::ClientSideInit()
{
	Super::ClientSideInit();


	// OnAscRegistered.Broadcast(AbilitySystemComponent);
	
	
	if (AHPPlayerController* CurrentPlayerController = GetHPPlayerController())
	{
		if (AHPHUD* HPHUD = Cast<AHPHUD>(CurrentPlayerController->GetHUD()))
		{
			HPHUD->InitOverlay(CurrentPlayerController, HPAbilitySystemComponent, HPAttributeSet, CombatComponent);
		}
	}


}

void AHPPlayerCharacter::HandleRespawn()
{
	//NextThingTODO: Basic Ability(ListenForEvent같은거)는 다시 시작하기
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//NextThingTODO: 빼야할 위젯 정해서 WidgetController에서 위젯 켜기

	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	if (HasAuthority())
	{
		HPAbilitySystemComponent->InitAbilityAndEffectAtRespawn();
		if (AControlPointGameMode* CPGameMode = Cast<AControlPointGameMode>(GetWorld()->GetAuthGameMode()))
		{
			//현재 거점 상태에 따라 특정 위치에서 생성
			ARespawnPlayerStart* RespawnPlayerStart = CPGameMode->GetRespawnPlayerStart(this);
			if (RespawnPlayerStart)
			{
				SetActorLocation(RespawnPlayerStart->GetActorLocation());
				RespawnPlayerStart->SetOccupied(false);
				UE_LOG(LogTemp, Warning,TEXT("Respawn at PlayerStart"));
				return;
			}
		}
		
			TWeakObjectPtr<AActor> StartSpot =  GetController()->StartSpot;
			if (StartSpot.IsValid())
			{
				SetActorLocation(StartSpot->GetActorLocation());
			}
		
	}
}

void AHPPlayerCharacter::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("HandleDeath"));
	DamageContributionComponent->SpreadKillLogs(HPAttributeSet->GetMaxHealth());
	PlayDeadAnimation();
	if (HPAbilitySystemComponent)
	{
		HPAbilitySystemComponent->CancelAllAbilities();
	}
	//NextThingTODO: 빼야할 위젯 정해서 WidgetController에서 처리하기

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHPPlayerCharacter::Death()
{
	HPAbilitySystemComponent->ApplyDeathEffect();
}


void AHPPlayerCharacter::ToggleMeleeHitBox_Implementation(bool TurnOn)
{
	MeleeHitBoxComponent->SetCollisionEnabled(TurnOn? ECollisionEnabled::QueryAndPhysics: ECollisionEnabled::NoCollision);
}

bool AHPPlayerCharacter::IsUsingServerRewind_Implementation() const
{
	return CombatComponent->IsUsingServerSideRewind();
}

FGenericTeamId AHPPlayerCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void AHPPlayerCharacter::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	TeamID = InTeamID;
}

void AHPPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHPPlayerCharacter, TeamID);
}

void AHPPlayerCharacter::TurnInPlace(float DeltaSeconds)
{
	//90도 넘으면 TurnInPlace
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::TurningRight;	
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::TurningLeft;
	}

	//돌고 있는 상태에서
	if (TurningInPlace != ETurningInPlace::NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaSeconds, 4.f);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.f) //15 offset 정도차이나면 멈추기
		{
			TurningInPlace = ETurningInPlace::NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AHPPlayerCharacter::RotateInPlace(float DeltaSeconds)
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
	
	// if (bDisableGameplay)
	// {
	// 	bUseControllerRotationYaw = false;
	// 	TurningInPlace = ETurningInPlace::_NotTurning;
	// 	return;
	// }
	
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaSeconds);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaSeconds;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

float AHPPlayerCharacter::CalculateXYSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;	//X,Y 값에 대한 값만 적용
	return Velocity.Size();
}

void AHPPlayerCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	
	//압축되어 양수가 된 각도 값 처리
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f,360.f);
		FVector2D OutRange(-90.f,0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AHPPlayerCharacter::SimulatedProxiesTurn()
{
	bRotateRootBone = false;
	float Speed = CalculateXYSpeed();

	//움직이면 TIP 적용되지 않도록
	if (Speed > 0.f) 
	{
		TurningInPlace = ETurningInPlace::NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::TurningRight;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::TurningLeft;
		}
		else
		{
			TurningInPlace = ETurningInPlace::NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::NotTurning;
}

void AHPPlayerCharacter::HideDesignatedBones()
{
	for (const FName& BoneName: DesignatedBoneNamesToHide)
	{
		GetMesh()->HideBoneByName(BoneName, EPhysBodyOp::PBO_None);
	}
}

void AHPPlayerCharacter::AimOffset(float DeltaSeconds)
{
	float Speed = CalculateXYSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) //멈춤 && 땅
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		
		if (TurningInPlace == ETurningInPlace::NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaSeconds);
	}
	if (Speed > 0.f || bIsInAir) // 뛰거나 점프
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::NotTurning;
	}
	CalculateAO_Pitch();
}

AHPPlayerController* AHPPlayerCharacter::GetHPPlayerController()
{
	if (!HPPlayerController)
	{
		HPPlayerController = Cast<AHPPlayerController>(GetController());
	}
	return HPPlayerController;
}

//Simulated Proxy와 로컬 플레이어 사이의 거리를 Timer의 주기대로 측정해 사라지거나 보이도록
void AHPPlayerCharacter::UpdateOverHeadWidgetVisibility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	
	if (LocalPlayerPawn)
	{
		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > OverHeadWidgetVisibilityRangeSquared);
	}
}

void AHPPlayerCharacter::ConfigureOverHeadWidget()
{
	if (!OverHeadWidgetComponent)
	{
		return;
	}


	if (GetController() && GetController()->IsLocalPlayerController())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return;
	}

	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	
	if (UHPOverheadWidget* HPOverheadUserWidget = Cast<UHPOverheadWidget>(OverHeadWidgetComponent->GetUserWidgetObject()))
	{
		BindCallbacksToDependencies();
		HPOverheadUserWidget->SetWidgetController(this);
		BroadcastInitialValues();
		OverHeadWidgetComponent->SetHiddenInGame(false);
		OverHeadWidgetComponent->SetTwoSided(true);
		ETeamAttitude::Type OpponentAttitude = FGenericTeamId::GetAttitude(this, LocalPlayerPawn);
		if (OpponentAttitude == ETeamAttitude::Hostile)
		{
			HPOverheadUserWidget->SetColorDependingOnAttitude(false);
		}
		else
		{
			HPOverheadUserWidget->SetColorDependingOnAttitude(true);
		}

		GetWorldTimerManager().ClearTimer(OverHeadWidgetTimerHandle);
		GetWorldTimerManager().SetTimer(OverHeadWidgetTimerHandle, this, &AHPPlayerCharacter::UpdateOverHeadWidgetVisibility, OverHeadWidgetVisibilityPeriod, true);
	}

	
}

void AHPPlayerCharacter::OnHealBanTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount==1)
	{
		OnHealBanTagChangedDelegate.Broadcast(true);
	}
	else if (NewCount==0)
	{
		OnHealBanTagChangedDelegate.Broadcast(false);
	}
}

void AHPPlayerCharacter::BindCallbacksToDependencies()
{
	FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
	
	HPAbilitySystemComponent->RegisterGameplayTagEvent(GameplayTags.State_Debuff_HealBan, EGameplayTagEventType::NewOrRemoved).
	AddUObject(this, &AHPPlayerCharacter::OnHealBanTagChanged);

	HPAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HPAttributeSet->GetHealthAttribute()).
	AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnHealthChangedDelegate.Broadcast(Data.NewValue);
	}
	);
	
	HPAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HPAttributeSet->GetMaxHealthAttribute()).
	AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnMaxHealthChangedDelegate.Broadcast(Data.NewValue);
	}
	);
}

void AHPPlayerCharacter::BroadcastInitialValues()
{
	UE_LOG(LogTemp,Warning,TEXT("AHPPlayerCharacter::BroadcastInitialValues"));
	OnMaxHealthChangedDelegate.Broadcast(HPAttributeSet->GetMaxHealth());
	OnHealthChangedDelegate.Broadcast(HPAttributeSet->GetHealth());
	OnHealBanTagChangedDelegate.Broadcast(HPAbilitySystemComponent->HasMatchingGameplayTag(FHPGameplayTags::Get().State_Debuff_HealBan));
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/LagCompensationComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "HPGameplayTags.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void ULagCompensationComponent::ExplosionServerApplyValidHit_Implementation(/*AHPPlayerCharacter* HitCharacter,*/
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime,
	const FProjectileApplyEffectParams& ProjectileApplyEffectParams,
	const TArray<AHPPlayerCharacter*>& OverlappedCharacters)
{
	// FServerSideRewindResult Confirm = ExplosionServerSideRewind(/*HitCharacter,*/ TraceStart, InitialVelocity, HitTime);

	bool bHitConfirmed = false;
	/* NEXTTHINGTODO: 잘작동되면 함수로 묶기*/
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.ActorsToIgnore.Add(GetOwner());

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	
	if (PathResult.HitResult.bBlockingHit)
	{
		bHitConfirmed = true;
	}
	if (!bHitConfirmed)
		return;
	
	/* NEXTTHINGTODO: 잘작동되면 함수로 묶기*/

	
	if (HPCharacter && bHitConfirmed)
	{
		UAbilitySystemComponent* SourceASC = ProjectileApplyEffectParams.SourceASC;
		TSet<AHPPlayerCharacter*> AppliedActorSet;
		for (AHPPlayerCharacter* OverlappedCharacter:OverlappedCharacters)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedCharacter);

			if (SourceASC && TargetASC)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s Hit!"), *OverlappedCharacter->GetName());
				float Damage = ProjectileApplyEffectParams.Damage;
				float AdditionalEffectValue  = ProjectileApplyEffectParams.AdditionalEffectValue;

				FVector_NetQuantize HitLocation = FVector::ZeroVector;
				FServerSideRewindResult ConfirmPerOverlappedActor = ExplosionServerSideRewind(OverlappedCharacter,ProjectileApplyEffectParams.OriginLocation, ProjectileApplyEffectParams.InnerRadius, ProjectileApplyEffectParams.OuterRadius, HitTime,HitLocation);

				if (!ConfirmPerOverlappedActor.bHitConfirmed) //닿지 않았으면 continue;
					continue;
				
				AppliedActorSet.Add(OverlappedCharacter);
				float FinalDamage = Damage;
				float PushPower = ProjectileApplyEffectParams.PushPower;
				if (ProjectileApplyEffectParams.bDistanceFalloff)
				{
					float DistSquared = FVector::DistSquared(ProjectileApplyEffectParams.OriginLocation,HitLocation);
					FinalDamage *= (1-DistSquared/(ProjectileApplyEffectParams.OuterRadius*ProjectileApplyEffectParams.OuterRadius));
					PushPower/=(ProjectileApplyEffectParams.OuterRadius/1000);
				}

				if (ProjectileApplyEffectParams.DamageEffectClass)
				{
					FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				
					FGameplayEffectSpecHandle DamageSpecHandle = SourceASC->MakeOutgoingSpec(
						ProjectileApplyEffectParams.DamageEffectClass,
						1,
						Context
						);
					UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, FHPGameplayTags::Get().SetByCaller_IncomingDamage, FinalDamage);
					TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data);
					
				}
				//밀어낼 수 있다면
				if (ProjectileApplyEffectParams.bCanPush)
				{
					FVector HitVelocity  = (ProjectileApplyEffectParams.OriginLocation-HitLocation)*PushPower;
					OverlappedCharacter->LaunchCharacter(HitVelocity, true,true);
				}
				
				if (ProjectileApplyEffectParams.AdditionalEffectClass)
				{
					float FinalValue = AdditionalEffectValue;
					FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				
					FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(
						ProjectileApplyEffectParams.AdditionalEffectClass,
						1,
						Context
						);


					if (ProjectileApplyEffectParams.bAdditionalEffectForTeam)
					{
						
						if (OverlappedCharacter->GetGenericTeamId()==ProjectileApplyEffectParams.GenericTeamId)
						{
							UE_LOG(LogTemp, Warning, TEXT("OverlappedCharacterTeam: %d, BombTeam: %d"), OverlappedCharacter->GetGenericTeamId().GetId(), ProjectileApplyEffectParams.GenericTeamId.GetId());
							UE_LOG(LogTemp, Warning, TEXT("MyTeam: %s"), *OverlappedCharacter->GetName());
							//NEXTHINGTODO: Additional일 경우 SetByCaller태그도 같이 가져오는 방안 일단 생각 해보기
							UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FHPGameplayTags::Get().SetByCaller_IncomingDamage, FinalValue);
							TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
						}
					}
				}
			
			}
		}
		
	}
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("LagComp BeginPlay: %s"),
		   IsComponentTickEnabled() ? TEXT("Tick Enabled") : TEXT("Tick Disabled"));	
}

void ULagCompensationComponent::CaptureFramePackage(FFramePackage& Package)
{
	if (!HPCharacter)
	{
		HPCharacter = Cast<AHPPlayerCharacter>(GetOwner());
	}
	
	if (HPCharacter)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = HPCharacter;
		for (auto& BoxPair : HPCharacter->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;

		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, AHPPlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);	//현 프레임 저장
	
	MoveBoxes(HitCharacter, Package);	//매개변수로 들어온 시간에 맞춰 박스 옮기기
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);	//SSR 박스판정 위해 스켈레탈 메시 끄기

	//헤드샷인지 확인하기 위해 헤드 박스 먼저 켜기
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block); //NEXTTHINGTODO: ECC_HitBox

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility//NEXTTHINGTODO: ECC_HitBox
		);
		
		if (ConfirmHitResult.bBlockingHit) //헤드샷 확정
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);//현 프레임 되돌리기
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else //헤드샷이 아닐 경우 다른 박스들 확인
		{
			//박스들 콜리전 모두 켜서
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);//NEXTTHINGTODO: ECC_HitBox
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_Visibility //NEXTTHINGTODO: ECC_HitBox
			);
			
			//하나라도 맞았다면 리턴
			if (ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxes(HitCharacter, CurrentFrame); //SSR 히트 박스 리셋
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package,
	AHPPlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.ActorsToIgnore.Add(GetOwner());

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	// Enable collision for the head first
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//NEXTTHINGTODO: ECC_HitBox만들기
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	
	if (PathResult.HitResult.bBlockingHit) // we hit the head, return early
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{ true, true };
	}
	else // we didn't hit the head; check the rest of the boxes
	{
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
			}
		}

		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, false };
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

FServerSideRewindResult ULagCompensationComponent::ExplosionConfirmHit(const FFramePackage& Package,
	AHPPlayerCharacter* HitCharacter,const FVector_NetQuantize& OriginLocation, float InnerRadius, float OuterRadius, float HitTime, FVector_NetQuantize& HitLocation)
{
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	//박스 다 켜기
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//NEXTTHINGTODO: ECC_HitBox만들기
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
			
		}		
	}

	//Sphere만들어서 박스가 폭발범위에 닿았는지 확인
	UWorld* World = GetWorld();
	TArray<UPrimitiveComponent*> OverlappedComponents;

	TArray<AActor*> ActorsToIgnore;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Visibility));//NEXTTHINGTODO: 채널 바꾸기
	
	const bool bOverlapped =
		UKismetSystemLibrary::SphereOverlapComponents(
			GetWorld(),
			OriginLocation,
			OuterRadius,
			ObjectTypes,
			UBoxComponent::StaticClass(), // 박스 컴포넌트만
			ActorsToIgnore,
			OverlappedComponents
		);

	bool bHitConfirmed = false;
	//닿은 것이 확인된 첫 번째 박스 구해서 맞았는지 확인

	for (auto& OverlapBox: OverlappedComponents)
	{
		bHitConfirmed = true;
		HitLocation = OverlapBox->GetComponentLocation();
		break;
	}
	if (bHitConfirmed)
	{
		DrawDebugSphere(
			GetWorld(),
			OriginLocation,   // 중심
			OuterRadius,   // 반지름
			32,                // Segments
			FColor::Red,
			false,             // PersistentLines
			2.f,               // LifeTime
			0,
				2.f                // Thickness
			);
	}
	// 	
	// ResetHitBoxes(HitCharacter, CurrentFrame);
	// EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	
	return FServerSideRewindResult{ bHitConfirmed, false };
}

void ULagCompensationComponent::CacheBoxPositions(AHPPlayerCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

//박스를 지정한 프레임에 맞게 움직이는 함수
void ULagCompensationComponent::MoveBoxes(AHPPlayerCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (Package.HitBoxInfo.Num()>0)
		{
			UE_LOG(LogTemp, Warning, TEXT("FIND!"));
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
				HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
				HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Can't FIND Current Package HitBoxInfo!"));
			for (auto& hitboxInfo:Package.HitBoxInfo)
			{
				UE_LOG(LogTemp, Warning, TEXT("hitboxInfo.Value: %s, %s, %s"), *hitboxInfo.Value.BoxExtent.ToString(), *hitboxInfo.Value.Location.ToString(), *hitboxInfo.Value.Rotation.ToString());
			}
			break;
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(AHPPlayerCharacter* HitCharacter, const FFramePackage& Package)
{
	//원하는 프레임으로 되돌리는 것에 사용
	if (HitCharacter == nullptr) return;
	for (TPair<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(AHPPlayerCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	//원래 판정인 SkeletalMesh판정으로
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled); 
	}
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (HPCharacter == nullptr || !HPCharacter->HasAuthority()) return;

	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		CaptureFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		CaptureFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(AHPPlayerCharacter* HitCharacter, float HitTime)
{
	bool bReturn =	//맞은 캐릭터가 null이거나, LagCompensationComponent가 없거나, 저장된 Frame의 Head나 Tail이 없으면 빠른 return
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	
	if (bReturn) return FFramePackage();
	
	//맞았는지 확인하기 위한 Frame package
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	// 맞은 캐릭터의 Frame 기록 
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	//보간이 필요 없는 경우들
	if (OldestHistoryTime > HitTime) //맞은 시간이 가장 오래된 기록의 시간보다 더 오래됐다면 빈 return
	{
		return FFramePackage();
	}

	if (OldestHistoryTime == HitTime) //맞은 시간이 정확히 가장 오래된 기록의 시간과 같다면 제일 오래된 기록 return
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime) //맞은 시간이 가장 최근의 시간과 같거나 더 최근이라면 제일 최근 기록 return
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}
	
	//보간 시작
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	//HitTime이 들어있는 프레임 찾기
	while (Older->GetValue().Time > HitTime)  
	{
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime) //보간이 필요없는지 한 번 더 확인
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	
	if (bShouldInterpolate)
	{
		//프레임 사이 히트 프레임 보간
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}


void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SaveFramePackage();
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

// FServerSideRewindResult ULagCompensationComponent::ServerCheckValidHit_Implementation(AHPPlayerCharacter* HitCharacter,
// 	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
// {
// 	return ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
// }
//

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(AHPPlayerCharacter* HitCharacter,
                                                                    const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(AHPPlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FServerSideRewindResult ULagCompensationComponent::ExplosionServerSideRewind(AHPPlayerCharacter* HitCharacter, const FVector_NetQuantize& OriginLocation, float InnerRadius, float OuterRadius, float HitTime, FVector_NetQuantize& HitLocation)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ExplosionConfirmHit(FrameToCheck, HitCharacter, OriginLocation, InnerRadius,OuterRadius, HitTime, HitLocation);
}


void ULagCompensationComponent::ProjectileServerApplyValidHit_Implementation(AHPPlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime,
	const FProjectileApplyEffectParams& ProjectileApplyEffectParams)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);

	bool bHeadShot = Confirm.bHeadShot;
	bool bHitConfirmed = Confirm.bHitConfirmed;
	
	if (HPCharacter && HitCharacter && Confirm.bHitConfirmed)
	{
		UAbilitySystemComponent* SourceASC = ProjectileApplyEffectParams.SourceASC;
		UAbilitySystemComponent* TargetASC = ProjectileApplyEffectParams.TargetASC;

		if (SourceASC && TargetASC)
		{
			float Damage = ProjectileApplyEffectParams.Damage;
			float AdditionalEffectValue  = ProjectileApplyEffectParams.AdditionalEffectValue;
			bHeadShot &= ProjectileApplyEffectParams.bCanHeadShot;
			
			if (ProjectileApplyEffectParams.DamageEffectClass)
			{
				float FinalDamage = Damage;
				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				
				FGameplayEffectSpecHandle DamageSpecHandle = SourceASC->MakeOutgoingSpec(
					ProjectileApplyEffectParams.DamageEffectClass,
					1,
					Context
					);
				if (bHeadShot)
				{
					FinalDamage*=2;
				}
				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, FHPGameplayTags::Get().SetByCaller_IncomingDamage, FinalDamage);
				TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data);
			}
			if (ProjectileApplyEffectParams.AdditionalEffectClass)
			{
				float FinalValue = AdditionalEffectValue;
				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				
				FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(
					ProjectileApplyEffectParams.AdditionalEffectClass,
					1,
					Context
					);

				//NEXTHINGTODO: Additional일 경우 SetByCaller태그도 같이 가져오는 방안 일단 생각 해보기
				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FHPGameplayTags::Get().SetByCaller_IncomingDamage, FinalValue);
				TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
			}
			
		}
	}
}
//
// void ULagCompensationComponent::ExplosionServerApplyValidHit_Implementation(AHPPlayerCharacter* HitCharacter,
// 	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime,
// 	const FProjectileApplyEffectParams& ProjectileApplyEffectParams)
// {
// 	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
// 	
// 	bool bHitConfirmed = Confirm.bHitConfirmed;
// 	
// 	if (HPCharacter && HitCharacter && bHitConfirmed)
// 	{
// 		UAbilitySystemComponent* SourceASC = ProjectileApplyEffectParams.SourceASC;
// 		UAbilitySystemComponent* TargetASC = ProjectileApplyEffectParams.TargetASC;
//
// 		if (SourceASC && TargetASC)
// 		{
// 			//방사형 펼치기,
// 			float Damage = ProjectileApplyEffectParams.Damage;
// 			float AdditionalEffectValue  = ProjectileApplyEffectParams.AdditionalEffectValue;
// 			
// 			if (ProjectileApplyEffectParams.DamageEffectClass)
// 			{
// 				float FinalDamage = Damage;
// 				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
// 				
// 				FGameplayEffectSpecHandle DamageSpecHandle = SourceASC->MakeOutgoingSpec(
// 					ProjectileApplyEffectParams.DamageEffectClass,
// 					1,
// 					Context
// 					);
//
// 				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, FHPGameplayTags::Get().SetByCaller_IncomingDamage, FinalDamage);
// 				TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data);
// 			}
// 			if (ProjectileApplyEffectParams.AdditionalEffectClass)
// 			{
// 				float FinalValue = AdditionalEffectValue;
// 				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
// 				
// 				FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(
// 					ProjectileApplyEffectParams.AdditionalEffectClass,
// 					1,
// 					Context
// 					);
//
// 				//NEXTHINGTODO: Additional일 경우 SetByCaller태그도 같이 가져오는 방안 일단 생각 해보기
// 				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FHPGameplayTags::Get().SetByCaller_IncomingDamage, FinalValue);
// 				TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
// 			}
// 			
// 		}
// 	}
// }

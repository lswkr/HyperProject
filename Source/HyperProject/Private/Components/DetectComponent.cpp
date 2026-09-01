// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DetectComponent.h"

#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "HPGameplayTags.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/SizeBox.h"
#include "Controller/HPPlayerController.h"
#include "Engine/OverlapResult.h"

UDetectComponent::UDetectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UDetectComponent::InitComponent(UAbilitySystemComponent* InASC, AHPPlayerController* InPC, AHPPlayerCharacter* InPlayerCharacter)
{
	AbilitySystemComponent = InASC;
	PlayerController = InPC;
	PlayerCharacter= InPlayerCharacter;
	
	bInitialized=true;
}

AActor* UDetectComponent::GetConfirmedActor() const
{
	if (ConfirmedActor!=nullptr)
		return ConfirmedActor;
	return nullptr;
}


void UDetectComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UDetectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitialized) //초기화가 다 되면
		return;

	//플레이어 컨트롤러가 있어야 하고
	if (!PlayerController)
		return;
	//로컬이어야 한다.
	if (!PlayerCharacter|| !PlayerCharacter->IsLocallyControlled())
		return;

	if (!DetectAbilityTag.IsValid() || !AbilitySystemComponent->HasMatchingGameplayTag(DetectAbilityTag)) //궁극기 충전이 되면
	{
		return;
	}
	
	FVector ViewLocation;
	FRotator ViewRotation;
	
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector BoxCenter = ViewLocation + ViewRotation.Vector() * (DetectingBoxExtent.X * 0.5f);
	TArray<FOverlapResult> OverlapResults;

	FCollisionQueryParams BoxParams(SCENE_QUERY_STAT(ApplyRadialDamage),  false, GetOwner());
	FCollisionObjectQueryParams ObjectParams(ECC_Pawn);
	
	GetWorld()->OverlapMultiByObjectType
	(
	OverlapResults, 
	BoxCenter, 
	ViewRotation.Quaternion(),
	ObjectParams,
	FCollisionShape::MakeBox(DetectingBoxExtent/2), 
	BoxParams
	);
	
	//ShowDebugBox(BoxCenter);
	TArray<AActor*> TargetCandidates;
	
	for (FOverlapResult& OverlapResult : OverlapResults)
	{
		if (IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface> (OverlapResult.GetActor()))
		{
			if (TeamAgentInterface->GetGenericTeamId() == PlayerController->GetGenericTeamId())
			{
				TargetCandidates.AddUnique(OverlapResult.GetActor());
			}
		}
	}
	if (TargetCandidates.Num() == 0) //사정 거리 내에 아무 것도 없다면 초기화
	{
		ConfirmedActor = nullptr;
		EraseTargetLockWidget();
	
		if (FGameplayAbilitySpec* AbilitySpec =  AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle))
		{
			if (AbilitySpec->IsActive())
				AbilitySystemComponent->CancelAbilityHandle(AbilitySpecHandle);
		}
		
		return;
	}

	float MinLengthSquared = FLT_MAX;
	AActor* TargetActor = nullptr;
	for (AActor* TargetCandidate:TargetCandidates)
	{
		float DistSquared = FVector::DistSquared(ViewLocation, TargetCandidate->GetActorLocation());
		if (MinLengthSquared > DistSquared)
		{
			TargetActor = TargetCandidate;
			MinLengthSquared = DistSquared;
		}
	}
	
	//지정된 액터가 비어있으면 새로 지정
	if (ConfirmedActor == nullptr)
	{
		ConfirmedActor = TargetActor;
	}
	//만약 지정된 액터가 현재 타겟이 된 액터와 다르다면 갈아 끼우기
	else
	{
		if (ConfirmedActor != TargetActor)
		{
			ConfirmedActor = TargetActor;
		}
	}
	DrawTargetLockWidget();
	SetTargetLockWidgetPosition();
	
	if (!AbilitySpecHandle.IsValid()) //SpecHandle이 없으면
	{
		AbilitySystemComponent->TryActivateAbilityByClass(GameplayAbilityUsingThisComponent);
	}
	else
	{
		if (FGameplayAbilitySpec* AbilitySpec =  AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle))
		{
			if (!AbilitySpec->IsActive())
				AbilitySystemComponent->TryActivateAbility(AbilitySpecHandle);
		}
	}
}

void UDetectComponent::ShowDebugBox(const FVector& InBoxCenter)
{
	if (!bTurnOnDebugBox)
		return;
	DrawDebugBox(GetWorld(),InBoxCenter, DetectingBoxExtent, FColor::Red, false, 3);
}

void UDetectComponent::DrawTargetLockWidget()
{
	if (!DrawnTargetLockWidget)
	{
		checkf(TargetLockWidgetClass, TEXT("Forgot to assign a valid widget class in Blueprint"));

		DrawnTargetLockWidget = CreateWidget<UUserWidget>(PlayerController, TargetLockWidgetClass);

		DrawnTargetLockWidget->AddToViewport();
	}
}

void UDetectComponent::SetTargetLockWidgetPosition()
{
	if (!DrawnTargetLockWidget  || !ConfirmedActor)
	{
		return;
	}
	
	FVector2D ScreenPosition;
	UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		PlayerController,
		ConfirmedActor->GetActorLocation(),
		ScreenPosition,
		true
	);

	if (TargetLockWidgetSize == FVector2D::ZeroVector)
	{
		DrawnTargetLockWidget->WidgetTree->ForEachWidget(
			[this](UWidget* FoundWidget)
			{
				if (USizeBox* FoundSizeBox = Cast<USizeBox>(FoundWidget))
				{
					TargetLockWidgetSize.X = FoundSizeBox->GetWidthOverride();
					TargetLockWidgetSize.Y = FoundSizeBox->GetHeightOverride();
				}
			}
		);
	}

	ScreenPosition -= (TargetLockWidgetSize / 2.f);
	DrawnTargetLockWidget->SetPositionInViewport(ScreenPosition,false);
}

void UDetectComponent::EraseTargetLockWidget()
{
	if (DrawnTargetLockWidget)
	{
		DrawnTargetLockWidget->RemoveFromParent();
		TargetLockWidgetSize = FVector2D::ZeroVector;
		DrawnTargetLockWidget = nullptr;
	}
}

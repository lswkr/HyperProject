// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "DetectComponent.generated.h"

class AHPPlayerCharacter;
class UAbilitySystemComponent;
class AHPPlayerController;
class UGameplayAbility;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HYPERPROJECT_API UDetectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDetectComponent();

	UFUNCTION(BlueprintCallable, Category = "Detect Component")
	void InitComponent(UAbilitySystemComponent* InASC, AHPPlayerController* InPC);

	AActor* GetConfirmedActor() const;
		
protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	AHPPlayerController* PlayerController;
	
	bool bInitialized = false;

	bool bHasTargets = false;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> GameplayAbilityUsingThisComponent;

	FGameplayAbilitySpecHandle AbilitySpecHandle;
	
	UPROPERTY(EditAnywhere)
	FVector DetectingBoxExtent;

	UPROPERTY(EditAnywhere)
	bool bTurnOnDebugBox;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DetectAbilityTag;
	
	void ShowDebugBox(const FVector& InBoxCenter);

	UPROPERTY()
	AActor* ConfirmedActor;

	UPROPERTY(EditDefaultsOnly, Category = "Detect Component")
	TSubclassOf<UUserWidget> TargetLockWidgetClass;

	void DrawTargetLockWidget();
	void SetTargetLockWidgetPosition();

	UPROPERTY()
	UUserWidget* DrawnTargetLockWidget;

	FVector2D TargetLockWidgetSize;

	void EraseTargetLockWidget();
};

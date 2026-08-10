// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "AbilitySpawnableActor.generated.h"

class UAbilitySystemComponent;

UCLASS()
class HYPERPROJECT_API AAbilitySpawnableActor : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	AAbilitySpawnableActor();

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamId) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void SetAbilitySystem(UAbilitySystemComponent* InASC);

protected:
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(Replicated)
	FGenericTeamId TeamID;



};

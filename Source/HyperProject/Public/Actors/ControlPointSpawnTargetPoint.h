// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "ControlPointSpawnTargetPoint.generated.h"

/**
 * 
 */

enum class EControlPointType:uint8;

UCLASS()
class HYPERPROJECT_API AControlPointSpawnTargetPoint : public ATargetPoint
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	EControlPointType TargetPointType;
};

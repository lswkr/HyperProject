// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HPBallisticProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/ProjectileMovementComponent.h"

// AHPBallisticProjectile::AHPBallisticProjectile()
// {
// 	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
//
// 	ProjectileMovement->bRotationFollowsVelocity = true;
// 	ProjectileMovement->SetIsReplicated(true);
// 	ProjectileMovement->InitialSpeed = InitialSpeed;
// 	ProjectileMovement->MaxSpeed = InitialSpeed;
// 	
// 	ProjectileMovement->ProjectileGravityScale = 1.f;
// }
//
//
// #if WITH_EDITOR
// void AHPBallisticProjectile::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
// {
// 	Super::PostEditChangeProperty(PropertyChangedEvent);
//
// 	FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
//
// 	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHPBallisticProjectile, InitialSpeed))
// 	{
// 		if (ProjectileMovement)
// 		{
// 			ProjectileMovement->InitialSpeed = InitialSpeed;
// 			ProjectileMovement->MaxSpeed = InitialSpeed;
// 		}
// 	}
//
// 	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHPBallisticProjectile, GravityScale))
// 	{
// 		if (ProjectileMovement)
// 		{
// 			ProjectileMovement->ProjectileGravityScale = GravityScale;
// 		}
// 	}
// }
// #endif
//
// void AHPBallisticProjectile::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
// 	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
// {
// 	UAbilitySystemComponent* SourceASC = ProjectileParams.SourceASC;
//
// 	if (!SourceASC)
// 		return;
//
// 	
// 	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
//
// 	if (!TargetASC)
// 		return;
// 	UE_LOG(LogTemp, Warning, TEXT("BOXHIT!"));
// 	FGameplayEffectContextHandle ContextHandle =  SourceASC->MakeEffectContext();
// 	
// 	FGameplayEffectSpecHandle ProjectileDamageSpec = ProjectileParams.SourceASC->MakeOutgoingSpec(DamageEffectClass, 1, ContextHandle);
//
// 	if (!ProjectileDamageSpec.Data)
// 		return;
// 		
// 	TargetASC->ApplyGameplayEffectSpecToSelf(*ProjectileDamageSpec.Data);
// 	
// 	if (AdditionalEffectClass)
// 	{
// 		FGameplayEffectSpecHandle AdditionalSpec = ProjectileParams.SourceASC->MakeOutgoingSpec(AdditionalEffectClass, 1, ContextHandle);
// 		TargetASC->ApplyGameplayEffectSpecToSelf(*AdditionalSpec.Data);
// 	}
// 	
// 	Super::OnBoxComponentHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
// }

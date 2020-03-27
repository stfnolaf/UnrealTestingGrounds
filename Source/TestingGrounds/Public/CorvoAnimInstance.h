// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CorvoAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TESTINGGROUNDS_API UCorvoAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		bool Aiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		bool Throwing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		bool Waiting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		float DeltaYawBetweenPlayerDirectionAndKnife = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		bool KnifeCatchKnockback = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		float KnifeCatchKnockbackAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		bool Hanging = false;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		bool Knockback = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoomerangThrowing)
		float KnockbackAlpha = 0.0f;*/
	
};

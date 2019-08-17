// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlinkIndicator.generated.h"

UCLASS()
class TESTINGGROUNDS_API ABlinkIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlinkIndicator();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UStaticMeshComponent* teleportIndicator = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UStaticMeshComponent* climbIndicator = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetIndicatorVisibility(bool visible);

	void SetClimbVisibility(bool visible);

};

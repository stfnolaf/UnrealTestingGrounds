// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunning.h"
#include "Corvo.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

// Sets default values for this component's properties
UWallRunning::UWallRunning()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	wallRunTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Wall Run Timeline"));
	WallRunInterpFunction.BindUFunction(this, FName("WallRunUpdate"));

	smoothRunRightTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Smooth Run Right Timeline"));
	SmoothRunRightInterpFunction.BindUFunction(this, FName("SmoothRunRightUpdate"));

	smoothRunLeftTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Smooth Run Left Timeline"));
	SmoothRunLeftInterpFunction.BindUFunction(this, FName("SmoothRunLeftUpdate"));
}


// Called when the game starts
void UWallRunning::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<ACorvo>(GetOwner());
	if (player == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("PLAYER VARIABLE NOT INITIALIZED"));
	}
	else {
		player->GetMovementComponent()->SetPlaneConstraintEnabled(true);
	}

	// ...

	if (wallRunCurve) {
		wallRunTimeline->AddInterpFloat(wallRunCurve, WallRunInterpFunction, FName("Dummy Value"));
		wallRunTimeline->SetLooping(true);
		wallRunTimeline->SetIgnoreTimeDilation(true);
	}

	if (smoothRunCurve) {
		smoothRunRightTimeline->AddInterpFloat(smoothRunCurve, SmoothRunRightInterpFunction, FName("RotValue"));
		smoothRunRightTimeline->SetLooping(false);
		smoothRunRightTimeline->SetIgnoreTimeDilation(true);

		smoothRunLeftTimeline->AddInterpFloat(smoothRunCurve, SmoothRunLeftInterpFunction, FName("RotValue"));
		smoothRunLeftTimeline->SetLooping(false);
		smoothRunLeftTimeline->SetIgnoreTimeDilation(true);
	}
	
}


// Called every frame
void UWallRunning::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Fire raycasts to left and right of player
	FHitResult leftHit;
	FHitResult rightHit;
	FVector startLoc = player->GetActorLocation();
	FVector rightVect = player->GetActorRightVector();
	bool contactLeft = false;
	bool contactRight = false;
	FHitResult finalHitResult;
	bool finalHitIsLeft = false;
	contactLeft = GetWorld()->LineTraceSingleByChannel(leftHit, startLoc, startLoc - 75.0f * rightVect, ECollisionChannel::ECC_Visibility);
	contactRight = GetWorld()->LineTraceSingleByChannel(rightHit, startLoc, startLoc + 75.0f * rightVect, ECollisionChannel::ECC_Visibility);

	// If actor is tagged as a wall and the impact normal's z coordinate is less than 0.1f
	if (contactLeft)
		contactLeft = contactLeft && leftHit.ImpactNormal.Z < 0.1f && leftHit.ImpactNormal.Z >= -0.05f && leftHit.Actor->ActorHasTag("Wall");

	if (contactRight)
		contactRight = contactRight && rightHit.ImpactNormal.Z < 0.1f && rightHit.ImpactNormal.Z >= -0.05f && rightHit.Actor->ActorHasTag("Wall");
	// if both raycasts hit an acceptable surface, discard the further one
	if (contactLeft && contactRight) {
		if ((leftHit.ImpactPoint - startLoc).Size() < (rightHit.ImpactPoint - startLoc).Size()) {
			contactRight = false;
			finalHitIsLeft = true;
			finalHitResult = leftHit;
		}
		else {
			contactLeft = false;
			finalHitIsLeft = false;
			finalHitResult = rightHit;
		}
	}
	else {
		if (contactLeft) {
			finalHitIsLeft = true;
			finalHitResult = leftHit;
		}
		else if (contactRight) {
			finalHitIsLeft = false;
			finalHitResult = rightHit;
		}
	}

	// set wallrunning direction based on impact normal
	if (contactLeft || contactRight) {
		if(finalHitIsLeft)
			playerDir = FVector::CrossProduct(finalHitResult.ImpactNormal, FVector::UpVector);
		else
			playerDir = FVector::CrossProduct(finalHitResult.ImpactNormal, FVector::DownVector);
	}
	
	// if not in wallrunning state, enter wallrunning state (start wallrunning timeline)
	//if (!wallRunTimeline->IsPlaying())
		//wallRunTimeline->PlayFromStart();
}

void UWallRunning::WallRunUpdate(float val) {
	//lock player to plane, disable gravity, and move them along the wallrunning direction
}

void UWallRunning::SmoothRunRightUpdate(float val) {

}

void UWallRunning::SmoothRunLeftUpdate(float val) {

}


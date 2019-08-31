// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunning.h"
#include "Corvo.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

// Sets default values for this component's properties
UWallRunning::UWallRunning()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
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
}


// Called every frame
void UWallRunning::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (player->GetMovementComponent()->IsFalling())
		lastWall = nullptr;

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

		// if not in wallrunning state, enter wallrunning state (start wallrunning timeline)
		if (!onWall && player->GetMovementComponent()->IsFalling() && ((lastWall && finalHitResult.Actor.IsValid() && UKismetMathLibrary::EqualEqual_ObjectObject(lastWall, rightHit.Actor.Get())) || !lastWall)) {
			StartWallRunning();
		}
	}
	else {
		StopWallRunning();
	}

	if (onWall) {
		// TODO - this doesn't work
		if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputKeyTimeDown(EKeys::SpaceBar) > 0.0f)
			player->MyJump();

		// TODO - this should only run once
		if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputKeyTimeDown(EKeys::W) == 0.0f) {
			StopWallRunning();
			if (finalHitResult.Actor.IsValid()) {
				UE_LOG(LogTemp, Warning, TEXT("ON WALL: %s"), onWall ? TEXT("TRUE") : TEXT("FALSE"));
				lastWall = finalHitResult.Actor.Get();
			}
		}
	}
} // end of TickComponent()

void UWallRunning::StartWallRunning() {
	onWall = true;
	player->GetCharacterMovement()->GravityScale = 0.0f;
	player->GetCharacterMovement()->SetPlaneConstraintNormal(FVector::UpVector);
	player->LockRailMovement();
	player->DisableHorizontalMovement();
	player->ResetJumps();
}

void UWallRunning::StopWallRunning() {
	onWall = false;
	player->GetCharacterMovement()->GravityScale = 1.0f;
	player->GetCharacterMovement()->SetPlaneConstraintNormal(FVector::ZeroVector);
	player->UnlockRailMovement();
	player->EnableHorizontalMovement();
}
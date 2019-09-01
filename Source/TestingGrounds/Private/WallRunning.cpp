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

	if (!player->GetMovementComponent()->IsFalling() && player->IsOnGround()) {
		lastWall = nullptr;
	}

	// Fire raycasts to left and right of player
	FHitResult leftHit;
	FHitResult rightHit;
	FVector startLoc = player->GetActorLocation();
	FVector rightVect = player->GetActorRightVector();
	bool contactLeft = false;
	bool contactRight = false;
	FHitResult finalHitResult;
	bool finalHitIsLeft = false;
	
	contactLeft = GetWorld()->LineTraceSingleByChannel(leftHit, startLoc, startLoc - 55.0f * rightVect, ECollisionChannel::ECC_Visibility);
	contactRight = GetWorld()->LineTraceSingleByChannel(rightHit, startLoc, startLoc + 55.0f * rightVect, ECollisionChannel::ECC_Visibility);

	// If actor is tagged as a wall and the impact normal's z coordinate is less than 0.1f

	if (contactLeft) {
		contactLeft = contactLeft && leftHit.ImpactNormal.Z < 0.1f && leftHit.ImpactNormal.Z >= -0.05f && leftHit.Actor->ActorHasTag("Wall");
		contactLeft = contactLeft && (lastWall == nullptr || (lastWall != nullptr && leftHit.Actor.IsValid() && !UKismetMathLibrary::EqualEqual_ObjectObject(lastWall, leftHit.Actor.Get())));
	}

	if (contactRight) {
		contactRight = contactRight && rightHit.ImpactNormal.Z < 0.1f && rightHit.ImpactNormal.Z >= -0.05f && rightHit.Actor->ActorHasTag("Wall");
		contactRight = contactRight && (lastWall == nullptr || (lastWall != nullptr && rightHit.Actor.IsValid() && !UKismetMathLibrary::EqualEqual_ObjectObject(lastWall, rightHit.Actor.Get())));
	}

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

		player->SetRailDir(playerDir);
		// enter wallrunning state IF
		if (!onWall // we're not currently on a wall
			&& player->GetMovementComponent()->IsFalling() ) // we are in the air 
		{
			StartWallRunning();
		}
	}

	if (onWall) {

		if (finalHitIsLeft) {
			//rotate to left wall
		}
		else {
			//rotate to right wall
		}

		if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->WasInputKeyJustPressed(EKeys::SpaceBar)) {
			StopWallRunning();
			if (finalHitResult.Actor.IsValid()) {
				lastWall = finalHitResult.Actor.Get();
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("LASTWALL COULD NOT BE SET - WALLRUNNING::TICKCOMPONENT()"));
			}
			player->ResetJumps();
			player->MyJump();
		}

		if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputKeyTimeDown(EKeys::W) == 0.0f
			|| UKismetMathLibrary::RadiansToDegrees(UKismetMathLibrary::Acos(FVector::DotProduct(player->GetCamera()->GetComponentRotation().Vector().GetSafeNormal(), playerDir.GetSafeNormal()))) > 30.0f) {
			StopWallRunning();
			if (finalHitResult.Actor.IsValid()) {
				lastWall = finalHitResult.Actor.Get();
			}
		}

		if (!contactLeft && !contactRight) {
			StopWallRunning();
		}

		// keep player constant distance from wall
		FPlane wallPlane = FPlane(finalHitResult.ImpactPoint, finalHitResult.ImpactNormal);
		float dist = wallPlane.PlaneDot(player->GetActorLocation());

		if (dist < 45.0f) {
			// move player further from the wall
			player->SetActorLocation(player->GetActorLocation() + finalHitResult.ImpactNormal, true);
		}
		else if (dist > 45.0f) {
			// move player towards the wall
			player->SetActorLocation(player->GetActorLocation() - finalHitResult.ImpactNormal, true);
		}
	}
} // end of TickComponent()

void UWallRunning::StartWallRunning() {
	onWall = true;
	player->GetCharacterMovement()->GravityScale = 0.0f;
	player->GetCharacterMovement()->SetPlaneConstraintNormal(FVector::UpVector);
	player->GetCharacterMovement()->AirControl = 1.0f;
	player->GetCharacterMovement()->MaxWalkSpeed = 850.0f;
	player->LockRailMovement();
	player->DisableHorizontalMovement();
	player->ResetJumps();
	timeStartNewRotateAnim = UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("STARTING WALLRUN"));
}

void UWallRunning::StopWallRunning() {
	onWall = false;
	player->GetCharacterMovement()->GravityScale = 1.0f;
	player->GetCharacterMovement()->SetPlaneConstraintNormal(FVector::ZeroVector);
	player->GetCharacterMovement()->AirControl = 0.5f;
	player->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	player->UnlockRailMovement();
	player->EnableHorizontalMovement();
	timeStartNewRotateAnim = UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("STOPPING WALLRUN"));
}
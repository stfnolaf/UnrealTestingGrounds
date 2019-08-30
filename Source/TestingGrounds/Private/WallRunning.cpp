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

	// run line traces to find walls
	TArray<AActor*> list;
	list.Add(player);
	FVector startLoc = player->GetActorLocation();
	FVector rightVect = player->GetActorRightVector().GetSafeNormal();
	FHitResult rightHit;
	if (
		GetWorld()->LineTraceSingleByChannel(rightHit, startLoc, startLoc + 75.0f * rightVect, ECollisionChannel::ECC_Visibility) && rightHit.Actor->ActorHasTag("Wall")
		) {
		playerDir = FVector::CrossProduct(rightHit.ImpactNormal, FVector(0.0f, 0.0f, -1.0f));
		//DrawDebugLine(GetWorld(), player->GetActorLocation(), player->GetActorLocation() + 100.0f * playerDir, FColor::Red, true, 10.0f);
		if (!onWall && player->GetCharacterMovement()->IsFalling()) {
			SetOnWall(true);
			wallRunTimeline->PlayFromStart();
			UE_LOG(LogTemp, Warning, TEXT("%s"), onWall ? TEXT("ON WALL") : TEXT("NOT ON WALL"));
		}
	}
	else {
		SetOnWall(false);
	}

	FHitResult leftHit;
	if (
		GetWorld()->LineTraceSingleByChannel(leftHit, startLoc, startLoc + -75.0f * rightVect, ECollisionChannel::ECC_Visibility) && leftHit.Actor->ActorHasTag("Wall")
		) {
		playerDir = FVector::CrossProduct(leftHit.ImpactNormal, FVector(0.0f, 0.0f, 1.0f));
		if (!onWall && player->GetCharacterMovement()->IsFalling()) {
			SetOnWall(true);
			wallRunTimeline->PlayFromStart();
			UE_LOG(LogTemp, Warning, TEXT("%s"), onWall ? TEXT("ON WALL") : TEXT("NOT ON WALL"));
		}
	}
	else {
		SetOnWall(false);
	}

	DrawDebugLine(GetWorld(), startLoc, startLoc + 75.0f * rightVect, FColor::Red, false);
	DrawDebugLine(GetWorld(), startLoc, startLoc + -75.0f * rightVect, FColor::Red, false);
}

void UWallRunning::WallRunUpdate(float val) {
	if ((UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputKeyTimeDown(EKeys::W) > 0.0f
		|| UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputKeyTimeDown(EKeys::S) > 0.0f) && onWall) {
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, FString("AAAAAAAAAAA"), true);
		player->GetCharacterMovement()->GravityScale = 0.0f;
		player->GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 1.0f));
		player->GetCharacterMovement()->AddForce(playerDir * 20000.0f);
	}
	else {
		wallRunTimeline->Stop();
		TurnOffWallRunning();
	}
}

void UWallRunning::SmoothRunRightUpdate(float val) {
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetControlRotation(
		FMath::Lerp(player->GetActorRotation(), 
		FRotator(player->GetActorRotation().Pitch, player->GetActorRotation().Yaw, -20.0f), 
		val)
	);
}

void UWallRunning::SmoothRunLeftUpdate(float val) {
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetControlRotation(
		FMath::Lerp(player->GetActorRotation(),
			FRotator(player->GetActorRotation().Pitch, player->GetActorRotation().Yaw, 20.0f),
			val)
	);
}

void UWallRunning::TurnOffWallRunning() {
	player->GetCharacterMovement()->GravityScale = 1.0f;
	player->GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 0.0f));
	SetOnWall(false);
}

void UWallRunning::SetOnWall(bool val) {
	onWall = val;
	if (val)
		player->DisableHorizontalMovement();
	else
		player->EnableHorizontalMovement();
}


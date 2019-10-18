// Fill out your copyright notice in the Description page of Project Settings.


#include "Grapple.h"
#include "Corvo.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "CableComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGrapple::UGrapple()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	player = Cast<ACorvo>(GetOwner());
	if (player == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("PLAYER VARIABLE NOT INITIALIZED"));
	}
}


// Called when the game starts
void UGrapple::BeginPlay()
{
	Super::BeginPlay();

	// ...
	myPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	player->GetCable()->SetVisibility(false);
	player->GetCable()->CableLength = 0.0f;
	player->GetCable()->CableWidth = 2.0f;
	player->GetCable()->EndLocation = FVector::ZeroVector;
	player->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UGrapple::OnHitWall);
}


// Called every frame
void UGrapple::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	deltaTime = DeltaTime;
}

void UGrapple::ShootCable(float deltaTime) {
	player->GetCable()->SetVisibility(true);
	timeShootingGrapple += deltaTime;
	if (timeShootingGrapple >= 0.5f)
		return;
	else
		player->GetCable()->SetWorldLocation(FMath::VInterpTo(player->GetCable()->GetComponentLocation(), hookLocation, deltaTime, 30.0f));
	UE_LOG(LogTemp, Warning, TEXT("REEEEEEEEEE"));
}

void UGrapple::OnInitiateAbility() {
	if (canGrapple) {
		canGrapple = false;
		grapplingHookEnabled = true;
		FHitResult cableHit;
		if (
			GetWorld()->LineTraceSingleByChannel(
				cableHit,
				player->GetCamera()->GetComponentLocation(),
				player->GetCamera()->GetComponentLocation() + player->GetCamera()->GetForwardVector() * grappleDistance,
				ECollisionChannel::ECC_Visibility)
			) {
			hookLocation = cableHit.Location;
			ShootCable(deltaTime);
			player->LaunchCharacter(FVector::UpVector * 500.0f, true, true);
			FLatentActionInfo info;
			info.CallbackTarget = this;
			info.ExecutionFunction = "GrappleAfterDelay";
			info.UUID = 123;
			info.Linkage = 0;
			UKismetSystemLibrary::Delay(this, 0.2f, info);
		}
		else {
			ResetGrapple();
		}
	}
}

void UGrapple::GrappleAfterDelay() {
	player->LaunchCharacter((hookLocation - player->GetActorLocation()) * 2.5f, false, false);
	FLatentActionInfo info;
	info.CallbackTarget = this;
	info.ExecutionFunction = "ResetGrapple";
	info.UUID = 125;
	info.Linkage = 0;
	UKismetSystemLibrary::Delay(this, 0.4f, info);
}

void UGrapple::ResetGrapple() {
	player->GetCable()->SetVisibility(false);
	player->GetCable()->SetWorldLocation(player->GetActorLocation());
	player->GetCable()->EndLocation = FVector::ZeroVector;
	grapplingHookEnabled = false;
	timeShootingGrapple = 0.0f;
	FLatentActionInfo info;
	info.CallbackTarget = this;
	info.ExecutionFunction = "ResetGrappleCoolDownAfterDelay";
	info.UUID = 124;
	info.Linkage = 0;
	UKismetSystemLibrary::Delay(this, grappleCooldown, info);
}

void UGrapple::ResetGrappleCoolDownAfterDelay() {
	canGrapple = true;
}

void UGrapple::OnHitWall(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherComp->IsSimulatingPhysics() && grapplingHookEnabled && Hit.ImpactNormal.Y < 0.1f) {
		player->LaunchCharacter(FVector::UpVector * 400.0f, true, true);
		UE_LOG(LogTemp, Warning, TEXT("Climbing Wall"));
	}
}

void UGrapple::OnReleaseAbility() {
	return;
}
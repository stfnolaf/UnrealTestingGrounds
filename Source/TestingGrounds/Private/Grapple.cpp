// Fill out your copyright notice in the Description page of Project Settings.


#include "Grapple.h"
#include "Corvo.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "CableComponent.h"

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
	player->GetCable()->EndLocation = FVector(120.0f, 10.0f, 60.0f);
}


// Called every frame
void UGrapple::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (hooked) {
		MovePlayer(DeltaTime);
		if (!hookMoveFinished) {
			//MOVE CABLE - make function?
			hookMoveFinished = MoveCable(DeltaTime);
		}
		else {
			player->GetCable()->SetWorldLocationAndRotation(
				FMath::VInterpTo(player->GetCable()->GetComponentLocation(), hookLocation, DeltaTime, 250.0f),
				UKismetMathLibrary::FindLookAtRotation(player->GetActorLocation(), hookLocation),
				false,
				nullptr,
				ETeleportType::None
			);
			if (canSetCableLength) {
				newCableLength = (player->GetActorLocation() - hookLocation).Size() + 50.0f;
				player->GetCable()->CableLength = newCableLength;
				player->GetCable()->EndLocation = FVector(15.0f, 0.0f, 30.0f);
				canSetCableLength = false;
			}
			if ((player->GetActorLocation() - hookLocation).Size() >= newCableLength + 400.0f) {
				ResetGrapple();
			}
		}
	}
}

bool UGrapple::MoveCable(float deltaTime) {
	player->GetCable()->SetVisibility(true);
	if ((player->GetCable()->GetComponentLocation() - hookLocation).Size() > 100.0f) {
		player->GetCable()->SetWorldLocation(FMath::VInterpTo(player->GetCable()->GetComponentLocation(), hookLocation, deltaTime, 10.0f));
		return false;
	}
	return true;
}

void UGrapple::MovePlayer(float deltaTime) {
	if (grappleButtonPressed) {
		player->LaunchCharacter((hookLocation - UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation()) * deltaTime * 250.0f, true, true);
		player->GetCable()->CableLength = (player->GetActorLocation() - hookLocation).Size();
	}
}

void UGrapple::OnInitiateAbility() {
	grappleButtonPressed = true;
	if (toggleGrapple) {
		TArray<AActor*> actorsToIgnore;
		FHitResult cableHit;
		actorsToIgnore.Add(player);
		TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
		objectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
		objectTypes.Add(EObjectTypeQuery::ObjectTypeQuery2);
		if (
			UKismetSystemLibrary::LineTraceSingleForObjects(
				GetWorld(),
				player->GetCamera()->GetComponentLocation(),
				player->GetCamera()->GetComponentLocation() + player->GetCamera()->GetForwardVector() * 3500.0f,
				objectTypes,
				false,
				actorsToIgnore,
				EDrawDebugTrace::ForDuration,
				cableHit,
				true,
				FLinearColor::Red,
				FLinearColor::Green,
				5.0f) && cableHit.Actor.IsValid()
			) {
			player->SetActorRotation(FRotator(UKismetMathLibrary::FindLookAtRotation(player->GetActorLocation(), cableHit.Location).Pitch, UKismetMathLibrary::FindLookAtRotation(player->GetActorLocation(), cableHit.Location).Yaw, 0.0f));
			hooked = true;
			hookLocation = cableHit.Location;
		}
		else {
			ResetGrapple();
		}
	}
	else {
		ResetGrapple();
	}
	toggleGrapple = !toggleGrapple;
}

void UGrapple::ResetGrapple() {
	hooked = false;
	hookMoveFinished = false;
	player->GetCable()->SetVisibility(false);
	player->GetCable()->SetWorldLocation(player->GetActorLocation());
	player->GetCable()->EndLocation = FVector(120.0f, 10.0f, 60.0f);
	canSetCableLength = true;
}

void UGrapple::OnReleaseAbility() {
	grappleButtonPressed = false;
	return;
}
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
}

void UGrapple::OnInitiateAbility() {
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
		player->SetActorRotation(FRotator(0.0f, UKismetMathLibrary::FindLookAtRotation(player->GetActorLocation(), cableHit.Location).Yaw, 0.0f));
		hooked = true;
		hookLocation = cableHit.Location;
	}
}

void UGrapple::OnReleaseAbility() {
	return;
}
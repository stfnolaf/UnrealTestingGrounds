// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunning.h"
#include "Corvo.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"

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
		wallDetector = player->GetWallDetector();
		if (wallDetector == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("WALL DETECTOR NOT INITIALIZED"));
		}
		else {
			wallDetector->OnComponentBeginOverlap.AddDynamic(this, &UWallRunning::OnOverlap);
			wallDetector->OnComponentEndOverlap.AddDynamic(this, &UWallRunning::OnOverlapEnd);
		}
	}

	// ...
	
}

void UWallRunning::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	player->ResetJumps();
	playerDir = player->GetCamera()->GetForwardVector();

	UE_LOG(LogTemp, Warning, TEXT("DETECTING OVERLAP WITH %s"), *(AActor::GetDebugName(OtherActor)));

	//TODO - INVESTIGATE WHETHER WE SHOULD CAST UPAWNMOVEMENTCOMPONENT TO UCHARACTERMOVEMENTCOMPONENT
	if (OtherActor->ActorHasTag("Wall") && player->GetMovementComponent()->IsFalling()) {
		//onWall = true;
		UE_LOG(LogTemp, Warning, TEXT("TOUCHING WALL"));
	}
}

void UWallRunning::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	UE_LOG(LogTemp, Warning, TEXT("ENDED OVERLAP WITH %s"), *(AActor::GetDebugName(OtherActor)));
}



// Called every frame
void UWallRunning::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


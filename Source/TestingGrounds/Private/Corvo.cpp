// Fill out your copyright notice in the Description page of Project Settings.


#include "Corvo.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
ACorvo::ACorvo()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
} // end of constructor

// Called when the game starts or when spawned
void ACorvo::BeginPlay()
{
	Super::BeginPlay();

	GetComponents(abilities);
} // end of BeginPlay()

void ACorvo::MoveForward(float Value) {
	if (Value != 0.0f) {
		AddMovementInput(GetActorForwardVector(), Value);
	}
} // end of MoveForward()

void ACorvo::OnInitiateAbility()
{
	if(activeAbility < abilities.Num())
		abilities[activeAbility]->OnInitiateAbility();
}

void ACorvo::OnReleaseAbility()
{
	if(activeAbility < abilities.Num())
		abilities[activeAbility]->OnReleaseAbility();
}

void ACorvo::OnQuit()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0), EQuitPreference::Quit, false);
}

void ACorvo::MoveRight(float Value) {
	if (Value != 0.0f) {
		AddMovementInput(GetActorRightVector(), Value);
	}
} // end of MoveRight()

// Called every frame
void ACorvo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACorvo::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACorvo::MyJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACorvo::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACorvo::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Teleport", IE_Pressed, this, &ACorvo::OnInitiateAbility);
	PlayerInputComponent->BindAction("Teleport", IE_Released, this, &ACorvo::OnReleaseAbility);

	PlayerInputComponent->BindAction("Quit", IE_Pressed, this, &ACorvo::OnQuit);

}

UStaticMeshComponent* ACorvo::GetHand() {
	return Hand;
}

UCameraComponent* ACorvo::GetCamera() {
	return myCamera;
}

void ACorvo::MyJump() {
	if (numJumps == 0 || numJumps == 1) {
		numJumps++;
		this->LaunchCharacter(FVector(0.0f, 0.0f, 420.0f), false, true);
	}
}

void ACorvo::Landed(const FHitResult& hit) {
	Super::Landed(hit);
	numJumps = 0;
}


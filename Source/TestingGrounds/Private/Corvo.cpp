// Fill out your copyright notice in the Description page of Project Settings.


#include "Corvo.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Public/TimerManager.h"
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

	arms->SetOnlyOwnerSee(true);
	
	animInst = Cast<UCorvoAnimInstance>(arms->GetAnimInstance());
	check(animInst);

	SpawnKnife();
} // end of BeginPlay()

void ACorvo::SpawnKnife() {
	if (knifeClass) {
		UWorld* world = GetWorld();
		if (world) {
			FActorSpawnParameters spawnParams;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			spawnParams.Owner = this;
			FRotator rotator = GetActorRotation();
			FVector spawnLocation = GetActorLocation();
			knife = world->SpawnActor<AKnife>(knifeClass, spawnLocation, rotator, spawnParams);
			knife->AttachToComponent(arms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("knife_socket"));
		}
	}
}

void ACorvo::MoveForward(float Value) {
	if (railMovementEnabled) {
		AddMovementInput(railDir, 1.0f);
		//UE_LOG(LogTemp, Warning, TEXT("Movement direction: %s"), *railDir.ToString());
	}
	else if (Value != 0.0f) {
		AddMovementInput(GetActorForwardVector(), Value);
	}
} // end of MoveForward()

void ACorvo::OnInitiateAbility()
{
	if(animInst != nullptr)
		animInst->Aiming = true;
}

void ACorvo::OnReleaseAbility()
{
	if(animInst != nullptr)
		animInst->Aiming = false;
}

void ACorvo::OnInitiateAttack() {
	if (animInst->Aiming) {
		if (hasKnife) {
			animInst->Throwing = true;
			FTimerDelegate waitTimerDelegate;
			waitTimerDelegate.BindUFunction(this, FName("EndWaitForKnife"));
			GetWorldTimerManager().SetTimer(knifeWaitHandle, waitTimerDelegate, 0.2f, false);
		}
	}
	if (!hasKnife && !knifeInAir) {
		RecallKnife();
	}
}

void ACorvo::ThrowKnife() {
	hasKnife = false;
	FHitResult hit;
	FVector handLoc = arms->GetSocketLocation(FName("knife_socket"));
	TArray<AActor*> list = TArray<AActor*>();
	list.Add(this);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), handLoc, handLoc + myCamera->GetForwardVector() * 10000.0f, ETraceTypeQuery::TraceTypeQuery1, false, list, EDrawDebugTrace::None, hit, true, FLinearColor::Red, FLinearColor::Green, 5.0f)) {
		fromLoc = knife->GetActorLocation();
		hitLoc = fromLoc - hit.ImpactPoint;
		UKismetMathLibrary::Vector_Normalize(hitLoc);
		hitLoc *= 15.0f;
		hitLoc += hit.ImpactPoint;
		heading = hitLoc - fromLoc;
		UKismetMathLibrary::Vector_Normalize(heading);
		rotation = knife->GetActorRotation();
		z = UKismetMathLibrary::FindLookAtRotation(knife->GetActorLocation(), hitLoc).Yaw;
		knifeInAir = true;
		knife->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
	}
}

void ACorvo::RecallKnife() {
	knife->GetActorRotation();
	knifeReturn = true;
	knifeInAir = true;
	halfDistance = UKismetMathLibrary::Vector_Distance(arms->GetSocketLocation(FName("knife_socket")), knife->GetActorLocation()) / 2.0f;
}

void ACorvo::EndWaitForKnife() {
	animInst->Waiting = false;
}

void ACorvo::OnReleaseAttack() {

}

void ACorvo::OnQuit()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0), EQuitPreference::Quit, false);
}

void ACorvo::MoveRight(float Value) {
	if (Value != 0.0f && horizontalMovementEnabled && !railMovementEnabled) {
		AddMovementInput(GetActorRightVector(), Value);
	}
} // end of MoveRight()

void ACorvo::AddYaw(float Value) {
	//if (railMovementEnabled) {
	//	mySpringArm->AddRelativeRotation(FRotator(mySpringArm->RelativeRotation.Pitch, Value, mySpringArm->RelativeRotation.Roll).Quaternion());
	//}
	//else {
		APawn::AddControllerYawInput(Value);
	//}
}

void ACorvo::AddPitch(float Value) {
	//if (railMovementEnabled) {
	//	mySpringArm->AddRelativeRotation(FRotator(Value * -1.0f, mySpringArm->RelativeRotation.Yaw, mySpringArm->RelativeRotation.Roll).Quaternion());
	//}
	//else {
		APawn::AddControllerPitchInput(Value);
	//}
}

// Called every frame
void ACorvo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (knifeInAir) {
		if (knifeReturn) {
			currDistance = UKismetMathLibrary::Vector_Distance(knife->GetActorLocation(), arms->GetSocketLocation(FName("knife_socket")));
			rotation = UKismetMathLibrary::RInterpTo_Constant(rotation, FRotator(rotation.Pitch + 10.0f, z, 90.0f), DeltaTime, 500.0f);
			knife->SetActorLocationAndRotation(
				UKismetMathLibrary::VInterpTo_Constant(
					knife->GetActorLocation(),
					myCamera->GetRightVector() * UKismetMathLibrary::Lerp(halfDistance, 0.0f, FMath::Clamp(FMath::Abs(halfDistance - currDistance) / (halfDistance / 2.0f), 0.0f, 1.0f)) + arms->GetSocketLocation(FName("knife_socket")),
					DeltaTime,
					currDistance <= 200.0f ? knifeSpeed * 0.25f : knifeSpeed * 0.125f
				),
				rotation,
				true
			);
			if (currDistance <= 10.0f) {
				knifeInAir = false;
				knifeReturn = false;
				hasKnife = true;
				animInst->Waiting = false;
				//animInst->Knockback = true;
				//UGameplayStatics::PlayWorldCameraShake(GetWorld(), CatchCameraShake, GetActorLocation(), 0.0f, 500.0f);
				knife->AttachToComponent(arms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("knife_socket"));
			}
		}
		else {
			rotation = UKismetMathLibrary::RInterpTo(rotation, UKismetMathLibrary::FindLookAtRotation(knife->GetActorLocation(), hitLoc).Add(0.0f, -90.0f, 0.0f), DeltaTime, knifeRotationSpeed);
			float distToHit = UKismetMathLibrary::Vector_Distance(knife->GetActorLocation(), hitLoc);
			if (distToHit <= 3.0f) {
				knifeInAir = false;
				animInst->Throwing = false;
			}
			else {
				knife->SetActorLocationAndRotation(
					UKismetMathLibrary::VInterpTo_Constant(knife->GetActorLocation(), hitLoc, DeltaTime, knifeSpeed),
					rotation,
					true
				);
			}
		}
	}
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

	PlayerInputComponent->BindAxis("Turn", this, &ACorvo::AddYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &ACorvo::AddPitch);

	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &ACorvo::OnInitiateAbility);
	PlayerInputComponent->BindAction("RightClick", IE_Released, this, &ACorvo::OnReleaseAbility);

	PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &ACorvo::OnInitiateAttack);
	PlayerInputComponent->BindAction("LeftClick", IE_Released, this, &ACorvo::OnReleaseAttack);

	PlayerInputComponent->BindAction("Quit", IE_Pressed, this, &ACorvo::OnQuit);

}

UStaticMeshComponent* ACorvo::GetHand() {
	return Hand;
}

UCameraComponent* ACorvo::GetCamera() {
	return myCamera;
}

void ACorvo::MyJump() {
	if (numJumps < maxJumps) {
		numJumps++;
		this->LaunchCharacter(FVector(0.0f, 0.0f, 420.0f), false, true);
		onGround = false;
	}
}

void ACorvo::DisableHorizontalMovement()
{
	horizontalMovementEnabled = false;
}

void ACorvo::EnableHorizontalMovement()
{
	horizontalMovementEnabled = true;
}

void ACorvo::Landed(const FHitResult& hit) {
	Super::Landed(hit);
	numJumps = 0;
	onGround = true;
}

void ACorvo::ResetJumps() {
	numJumps = 0;
}

void ACorvo::LockRailMovement() {
	railMovementEnabled = true;
	//mySpringArm->bInheritYaw = false;
	//mySpringArm->bInheritPitch = false;
	//mySpringArm->bUsePawnControlRotation = false;
}

void ACorvo::UnlockRailMovement() {
	railMovementEnabled = false;
	//mySpringArm->bInheritYaw = true;
	//mySpringArm->bInheritPitch = true;
	//mySpringArm->bUsePawnControlRotation = true;
}

void ACorvo::SetRailDir(FVector vect) {
	railDir = vect;
}

bool ACorvo::IsOnGround() {
	return onGround;
}

UCapsuleComponent* ACorvo::GetCapsuleComponent() {
	return myCapsule;
}
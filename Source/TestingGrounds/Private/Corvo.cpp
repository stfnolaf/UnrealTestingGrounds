// Fill out your copyright notice in the Description page of Project Settings.


#include "Corvo.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/SpringArmComponent.h"
#include "WallRunning.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"

// Sets default values
ACorvo::ACorvo()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Yaw Controller"));
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	SpringArm->bUsePawnControlRotation = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	PlayerCamera->FieldOfView = 106.0f;
	PlayerCamera->SetupAttachment(SpringArm);

	ArmController = CreateDefaultSubobject<USpringArmComponent>(TEXT("Pitch Controller"));
	ArmController->TargetArmLength = 0.0f;
	ArmController->SetupAttachment(PlayerCamera);
	ArmController->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));

	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms Mesh"));
	ArmsMesh->SetupAttachment(ArmController);
	ArmsMesh->SetRelativeLocation(FVector(-18.0f, 0.0f, -112.0f));
	ArmsMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	WallRunningComponent = CreateDefaultSubobject<UWallRunning>(TEXT("Wall Running"));

	SphereTracer = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Tracer"));
	SphereTracer->SetupAttachment(GetCapsuleComponent());
	SphereTracer->SetSphereRadius(100.0f);
	SphereTracer->OnComponentBeginOverlap.AddDynamic(this, &ACorvo::OnLedgeTracerOverlapBegin);
	SphereTracer->OnComponentEndOverlap.AddDynamic(this, &ACorvo::OnLedgeTracerOverlapEnd);
} // end of constructor

// Called when the game starts or when spawned
void ACorvo::BeginPlay()
{
	Super::BeginPlay();

	ArmsMesh->SetOnlyOwnerSee(true);
	
	animInst = Cast<UCorvoAnimInstance>(ArmsMesh->GetAnimInstance());
	check(animInst);

	SpawnKnife();
} // end of BeginPlay()

void ACorvo::OnLedgeTracerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	CanTraceForLedges = true;
}

void ACorvo::OnLedgeTracerOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	CanTraceForLedges = false;
}

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
			knife->AttachToComponent(ArmsMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("knife_socket"));
			knife->InitializeOwner(this);
			knifeThrown = false;
		}
	}
}

void ACorvo::MoveForward(float Value) {
	if (IsClimbingLedge) {
		return;
	}
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
	if(animInst != nullptr && !knifeThrown)
		animInst->Aiming = true;
}

void ACorvo::OnReleaseAbility()
{
	if(animInst != nullptr)
		animInst->Aiming = false;
}

void ACorvo::OnInitiateAttack() {
	if (animInst->Aiming && !knifeThrown) {
		animInst->Throwing = true;
	}
	else if (knifeThrown) {
		RecallKnife();
	}
}

void ACorvo::ThrowKnife() {
	if (!knifeThrown) {
		knife->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
		knife->Throw(PlayerCamera->GetComponentRotation(), PlayerCamera->GetForwardVector(), PlayerCamera->GetComponentLocation(), 3500.0f);
		knifeThrown = true;
		animInst->Throwing = false;
		animInst->Aiming = false;
	}
}

void ACorvo::RecallKnife() {
	if (knife->GetKnifeState() == EKnifeState::VE_LodgedInSomething || knife->GetKnifeState() == EKnifeState::VE_Launched) {
		if (knife->Recall())
			animInst->Waiting = true;
	}
	else if (knife->GetKnifeState() == EKnifeState::VE_PermaLodged) {
		GrappleToLocation(knife->GetActorLocation());
	}
	//knife->AttachToComponent(ArmsMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("knife_socket"));
}

void ACorvo::GrappleToLocation(FVector TargetLocation) {
	LaunchCharacter((TargetLocation - GetActorLocation()) * 1.5f, false, false);
}

void ACorvo::EndWaitForKnife() {
	animInst->Waiting = false;
	animInst->Throwing = false;
	animInst->Aiming = false;
	knifeThrown = false;
	knife->AttachToComponent(ArmsMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("knife_socket"));
	UGameplayStatics::PlayWorldCameraShake(GetWorld(), CatchCameraShake, GetActorLocation(), 0.0f, 500.0f);
}

void ACorvo::UpdateDeltaYawBetweenPlayerAndKnife() {
	animInst->DeltaYawBetweenPlayerDirectionAndKnife = FMath::FindDeltaAngleDegrees(PlayerCamera->GetForwardVector().Rotation().Yaw, (knife->GetActorLocation() - GetActorLocation()).Rotation().Yaw);
}

void ACorvo::OnReleaseAttack() {

}

void ACorvo::OnQuit()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0), EQuitPreference::Quit, false);
}

void ACorvo::MoveRight(float Value) {
	if (Value != 0.0f && horizontalMovementEnabled && !railMovementEnabled) {
		if (IsClimbingLedge) {
			AddMovementInput(FVector::CrossProduct(WallNormal, FVector::UpVector * Value) * 0.2f);
		}
		else {
			AddMovementInput(GetActorRightVector(), Value);
		}
	}
} // end of MoveRight()

void ACorvo::AddYaw(float Value) {
	APawn::AddControllerYawInput(Value);
}

void ACorvo::AddPitch(float Value) {
	APawn::AddControllerPitchInput(Value);
}

// Called every frame
void ACorvo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CanGrabLedge = false;

	if (CanTraceForLedges || IsClimbingLedge) {
		FHitResult WallHit;
		FHitResult LedgeHit;
		TArray<AActor*> IgnoreActors = TArray<AActor*>();
		IgnoreActors.Add(this);
		if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + 150.0f * FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0.0f), 20.0f, ETraceTypeQuery::TraceTypeQuery3, false, IgnoreActors, EDrawDebugTrace::ForOneFrame, WallHit, true)) {
			WallTraceImpact = WallHit.ImpactPoint;
			WallNormal = WallHit.ImpactNormal;
		}
		if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 500.0f) + GetActorForwardVector() * 70.0f, GetActorLocation() + GetActorForwardVector() * 70.0f, 20.0f, ETraceTypeQuery::TraceTypeQuery3, false, IgnoreActors, EDrawDebugTrace::ForOneFrame, LedgeHit, true)) {
			LedgeHeight = LedgeHit.ImpactPoint;
			if (!IsClimbingLedge) {
				if (GetCharacterMovement()->IsFalling() && !JustLetGo) {
					GrabLedge();
				}
				else {
					CanGrabLedge = true;
				}
			}
		}
	}

}

void ACorvo::LetGoOfLedge() {
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	IsClimbingLedge = false;
	JustLetGo = true;
	FTimerDelegate letGoTimerDeleg;
	letGoTimerDeleg.BindUFunction(this, FName("ResetJustLetGo"));
	GetOwner()->GetWorldTimerManager().SetTimer(LedgeGrabDelayHandle, letGoTimerDeleg, 1.0f, false);
	GetCharacterMovement()->BrakingDecelerationFlying = 0.0f;
}

void ACorvo::ResetJustLetGo() {
	JustLetGo = false;
}

void ACorvo::GrabLedge() {
	ResetJumps();
	IsClimbingLedge = true;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector TempVector = FVector(WallNormal * FVector(CapsuleRadius, CapsuleRadius, 0.0f));
	FVector RelativeLocation = FVector(TempVector.X + WallTraceImpact.X, TempVector.Y + WallTraceImpact.Y, LedgeHeight.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FLatentActionInfo info;
	info.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), RelativeLocation, UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), WallTraceImpact), false, false, 0.2f, true, EMoveComponentAction::Move, info);
	GetCharacterMovement()->StopMovementImmediately();
	CanGrabLedge = false;
	GetCharacterMovement()->BrakingDecelerationFlying = 1000.0f;
}

// Called to bind functionality to input
void ACorvo::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACorvo::SpacebarAction);
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

	PlayerInputComponent->BindAction("Slow", IE_Pressed, this, &ACorvo::ToggleTime);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACorvo::CrouchAction);

	PlayerInputComponent->BindAction("Quit", IE_Pressed, this, &ACorvo::OnQuit);

	this->PlayerInputComponent = PlayerInputComponent;

}

void ACorvo::CrouchAction() {
	if (IsClimbingLedge) {
		LetGoOfLedge();
	}
}

void ACorvo::ToggleTime() {
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), UGameplayStatics::GetGlobalTimeDilation(GetWorld()) == 0.25f ? 1.0f : 0.25f);
}

float ACorvo::GetForwardMovement() {
	return PlayerInputComponent->GetAxisValue(FName("MoveForward"));
}

USkeletalMeshComponent* ACorvo::GetMyMesh() {
	return ArmsMesh;
}

UCameraComponent* ACorvo::GetCamera() {
	return PlayerCamera;
}

UWallRunning* ACorvo::GetWallRunningComponent() {
	return WallRunningComponent;
}

void ACorvo::SpacebarAction() {
	if (CanGrabLedge) {
		GrabLedge();
		return;
	}
	if (numJumps < maxJumps) {
		numJumps++;
		this->LaunchCharacter(IsClimbingLedge ? ((WallNormal * 210.0f) + (FVector::UpVector * 420.0f)) : (FVector::UpVector * 420.0f), false, true);
		if (IsClimbingLedge)
			LetGoOfLedge();
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
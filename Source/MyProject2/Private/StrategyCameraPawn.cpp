// StrategyCameraPawn.cpp
#include "StrategyCameraPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

AStrategyCameraPawn::AStrategyCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true; // The controller will tick, not the pawn.

	bUseControllerRotationYaw = true;
	
	// Create a root component that we can attach things to.
	PawnRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PawnRoot"));
	SetRootComponent(PawnRootComponent);

	// Create a movement component that allows for simple flying/floating movement.
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->MaxSpeed = 3000.f; // You can adjust this default speed
	MovementComponent->Acceleration = 1500.f;
	MovementComponent->Deceleration = 3000.f;
	// --- THIS LINE IS REMOVED ---
	// MovementComponent->bPositionCorrected = true; // This is a protected member

	// Create a camera boom (spring arm)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 1500.0f;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5.0f;
	CameraBoom->bUsePawnControlRotation = false; // We control rotation via Controller + Boom Pitch
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = false;
	CameraBoom->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Create a follow camera
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ViewCamera->bUsePawnControlRotation = false;
}

void AStrategyCameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GFrameCounter % 120 == 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[PAWN] '%s' Tick: World Location: %s | World Rotation: %s"),
			*GetNameSafe(this),
			*GetActorLocation().ToString(),
			*GetActorRotation().ToString());
	}
}

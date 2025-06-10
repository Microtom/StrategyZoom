// StrategyPlayerController.cpp
#include "StrategyPlayerController.h"
#include "StrategyCameraPawn.h" // Include our new pawn
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

// --- CONSTRUCTOR: Gutted of camera components ---
AStrategyPlayerController::AStrategyPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	// Initialize pointers
	PossessedCameraPawn = nullptr;
	TargetZoomLength = 1500.f; // Set a sensible default
}

// --- ON POSSESS: The new place to initialize everything ---
void AStrategyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Log, TEXT("[CONTROLLER] OnPossess: Possessing Pawn '%s'."), *GetNameSafe(InPawn));

	// Cache a reference to the pawn we possessed
	PossessedCameraPawn = Cast<AStrategyCameraPawn>(InPawn);
	if (!PossessedCameraPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[CONTROLLER] OnPossess: FAILED to cast Pawn to AStrategyCameraPawn! Controls will not work."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[CONTROLLER] OnPossess: Successfully cast and cached PossessedCameraPawn."));

	// --- THIS IS THE CRITICAL ADDITION ---
	// Add Input Mapping Context
	if (DefaultMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogTemp, Log, TEXT("[CONTROLLER] OnPossess: Successfully added Mapping Context '%s' to subsystem."), *DefaultMappingContext->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[CONTROLLER] OnPossess: Failed to get EnhancedInputLocalPlayerSubsystem."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CONTROLLER] OnPossess: DefaultMappingContext is not set in the Blueprint! Cannot add to subsystem."));
	}

	// Set input mode to allow game and UI interaction
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
	UE_LOG(LogTemp, Log, TEXT("[CONTROLLER] OnPossess: Set Input Mode to GameAndUI."));
	// --- END OF CRITICAL ADDITION ---


	// Now that we have a pawn and input is set up, initialize its camera settings
	InitializeCameraSettings();
}

// --- TICK: Now operates on the pawn's camera boom ---
void AStrategyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PossessedCameraPawn)
	{
		// Log once every 120 frames if our pawn is missing
		if (GFrameCounter % 120 == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("[CONTROLLER] Tick: PossessedCameraPawn is NULL! Cannot update."));
		}
		return;
	}

	// Log a detailed status update every 120 frames
	if (GFrameCounter % 120 == 0)
	{
		USpringArmComponent* CameraBoom = PossessedCameraPawn->GetCameraBoom();
		UE_LOG(LogTemp, Verbose, TEXT("--- [CONTROLLER] Tick Status ---"));
		UE_LOG(LogTemp, Verbose, TEXT("  Controller Control Rotation: %s"), *GetControlRotation().ToString());
		UE_LOG(LogTemp, Verbose, TEXT("  Possessed Pawn Location: %s"), *PossessedCameraPawn->GetActorLocation().ToString());
		UE_LOG(LogTemp, Verbose, TEXT("  Possessed Pawn Rotation: %s"), *PossessedCameraPawn->GetActorRotation().ToString());
		if (CameraBoom)
		{
			UE_LOG(LogTemp, Verbose, TEXT("  Boom Target Length: %.2f (Controller) vs %.2f (Actual)"), TargetZoomLength, CameraBoom->TargetArmLength);
		}
		UE_LOG(LogTemp, Verbose, TEXT("---------------------------------"));
	}
	
	UpdateCameraZoomAndPitch(DeltaTime);
}

// --- SETUP INPUT COMPONENT: Stays mostly the same ---
void AStrategyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// (Your existing SetupInputComponent code for binding actions is correct and can stay here)
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (MoveAction) EnhancedInput->BindAction(MoveAction.Get(), ETriggerEvent::Triggered, this, &AStrategyPlayerController::HandleMoveInput);
        if (ZoomAction) EnhancedInput->BindAction(ZoomAction.Get(), ETriggerEvent::Triggered, this, &AStrategyPlayerController::HandleZoomInput);
        if (RotateActionTrigger) EnhancedInput->BindAction(RotateActionTrigger.Get(), ETriggerEvent::Triggered, this, &AStrategyPlayerController::HandleRotateCameraTrigger);
        if (RotateActionValue) EnhancedInput->BindAction(RotateActionValue.Get(), ETriggerEvent::Triggered, this, &AStrategyPlayerController::HandleRotateCameraValue);
    }
}

// --- INITIALIZE CAMERA: Now operates on the pawn's boom ---
void AStrategyPlayerController::InitializeCameraSettings()
{
	if (!PossessedCameraPawn) return;
	
	USpringArmComponent* CameraBoom = PossessedCameraPawn->GetCameraBoom();
	if (!CameraBoom) return;

	TargetZoomLength = FMath::Clamp(CameraBoom->TargetArmLength, MinZoomLength, MaxZoomLength);
	
	if (CameraPitchByZoomCurve)
	{
		const float CurveTargetPitchValue = CameraPitchByZoomCurve->GetFloatValue(TargetZoomLength);
		FRotator BoomNewRelativeRotation = CameraBoom->GetRelativeRotation();
		BoomNewRelativeRotation.Pitch = CurveTargetPitchValue * -1.f;
		CameraBoom->SetRelativeRotation(BoomNewRelativeRotation);
	}
}

// --- UPDATE ZOOM/PITCH: Now operates on the pawn's boom ---
void AStrategyPlayerController::UpdateCameraZoomAndPitch(float DeltaTime)
{
	if (!PossessedCameraPawn) return;
	USpringArmComponent* CameraBoom = PossessedCameraPawn->GetCameraBoom();
	if (!CameraBoom) return;

	// Interpolate Target Arm Length
	const float InterpolatedArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetZoomLength, DeltaTime, ZoomInterpSpeed);
	CameraBoom->TargetArmLength = InterpolatedArmLength;

	// Calculate and Interpolate Pitch based on Curve
	if (CameraPitchByZoomCurve)
	{
		const float TargetPitchValue = CameraPitchByZoomCurve->GetFloatValue(InterpolatedArmLength);
		FRotator CurrentBoomRotation = CameraBoom->GetRelativeRotation();
		CurrentBoomRotation.Pitch = FMath::RInterpTo(CurrentBoomRotation, FRotator(TargetPitchValue * -1.f, 0, 0), DeltaTime, RotationInterpSpeed).Pitch;
		CameraBoom->SetRelativeRotation(CurrentBoomRotation);
	}
}


// --- HANDLE MOVE INPUT: Now uses AddMovementInput on the pawn ---
void AStrategyPlayerController::HandleMoveInput(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Warning, TEXT("[CONTROLLER] HandleMoveInput TRIGGERED! Value: %s"), *MovementVector.ToString());

	if (!PossessedCameraPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[CONTROLLER] HandleMoveInput: Cannot move because PossessedCameraPawn is NULL."));
		return;
	}

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	UE_LOG(LogTemp, Log, TEXT("[CONTROLLER] HandleMoveInput: Calling AddMovementInput on Pawn."));
	PossessedCameraPawn->AddMovementInput(ForwardDirection, MovementVector.Y * CameraMoveSpeed);
	PossessedCameraPawn->AddMovementInput(RightDirection, MovementVector.X * CameraMoveSpeed);


}



// (HandleZoomInput, HandleRotateCameraTrigger, HandleRotateCameraValue, etc., do not need to change as they only modify controller state or call controller functions like AddYawInput)
// (Your existing correct versions of these functions are fine)



void AStrategyPlayerController::HandleZoomInput(const FInputActionValue& Value)
{
	const float ZoomAxisValue = Value.Get<float>();
	if (FMath::IsNearlyZero(ZoomAxisValue) || !PossessedCameraPawn)
	{
		return;
	}

	USpringArmComponent* CameraBoom = PossessedCameraPawn->GetCameraBoom();
	if (!CameraBoom)
	{
		return;
	}

	// --- Proactive Panning Calculation (ONLY for Zooming IN) ---
	if (ZoomAxisValue > 0)
	{
		FHitResult HitResult;
		if (GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult) && HitResult.bBlockingHit)
		{
			// 1. Calculate the direction to pan.
			const FVector FocusPoint = HitResult.Location;
			const FVector PawnLocation = PossessedCameraPawn->GetActorLocation();
			FVector PanDirection = FocusPoint - PawnLocation;
			PanDirection.Z = 0.f;
			PanDirection.Normalize();

			// 2. Calculate the horizontal distance of the camera from the pawn BEFORE the zoom.
			const float CurrentArmLength = CameraBoom->TargetArmLength;
			const float CurrentPitchRad = FMath::DegreesToRadians(CameraBoom->GetRelativeRotation().Pitch);
			const float HorizontalOffset_Before = CurrentArmLength * FMath::Cos(CurrentPitchRad);

			// 3. Calculate the new arm length and pitch AFTER the zoom.
			const float NewArmLength = FMath::Clamp(CurrentArmLength - ZoomAxisValue * ZoomStepAmount, MinZoomLength, MaxZoomLength);
			float NewPitchDeg = CameraBoom->GetRelativeRotation().Pitch;
			if (CameraPitchByZoomCurve)
			{
				// Get the new pitch from the curve and multiply by -1
				NewPitchDeg = CameraPitchByZoomCurve->GetFloatValue(NewArmLength) * -1.f;
			}
			const float NewPitchRad = FMath::DegreesToRadians(NewPitchDeg);

			// 4. Calculate the horizontal distance of the camera from the pawn AFTER the zoom.
			const float HorizontalOffset_After = NewArmLength * FMath::Cos(NewPitchRad);

			// 5. The magnitude of our pan is the change in the horizontal offset.
			const float PanMagnitude = HorizontalOffset_Before - HorizontalOffset_After;

			// 6. The final world offset is the direction towards the mouse, scaled by the correct magnitude.
			const FVector WorldOffset = PanDirection * PanMagnitude;
			
			// Instantly move the pawn by this calculated offset.
			PossessedCameraPawn->AddActorWorldOffset(WorldOffset, true);
		}
	}
	// For Zooming OUT, we do no special panning logic.

	// Finally, update the target zoom length for the Tick function to smoothly interpolate to.
	TargetZoomLength = FMath::Clamp(CameraBoom->TargetArmLength - ZoomAxisValue * ZoomStepAmount, MinZoomLength, MaxZoomLength);
}

void AStrategyPlayerController::HandleRotateCameraTrigger(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	UE_LOG(LogTemp, Warning, TEXT("HandleRotateCameraTrigger TRIGGERED! bPressed: %s. Current bIsRotatingCamera: %s"), bPressed ? TEXT("true") : TEXT("false"), bIsRotatingCamera ? TEXT("true") : TEXT("false"));
	bIsRotatingCamera = bPressed;

	if (bIsRotatingCamera)
	{
		if(GetMousePosition(LastMousePositionForRotation.X, LastMousePositionForRotation.Y))
		{
			UE_LOG(LogTemp, Log, TEXT("HandleRotateCameraTrigger: Rotation STARTED. LastMousePos: %s"), *LastMousePositionForRotation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HandleRotateCameraTrigger: Rotation STARTED but GetMousePosition failed!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("HandleRotateCameraTrigger: Rotation ENDED."));
	}
}

void AStrategyPlayerController::HandleRotateCameraValue(const FInputActionValue& Value)
{
	const FVector2D MouseDelta = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Warning, TEXT("HandleRotateCameraValue TRIGGERED! MouseDelta: X=%.2f, Y=%.2f. bIsRotatingCamera: %s"), MouseDelta.X, MouseDelta.Y, bIsRotatingCamera ? TEXT("true") : TEXT("false"));

	if (!bIsRotatingCamera)
	{
		UE_LOG(LogTemp, Log, TEXT("HandleRotateCameraValue: Not currently rotating. Ignoring value."));
		return;
	}
	if (MouseDelta.IsNearlyZero())
	{
		 UE_LOG(LogTemp, Log, TEXT("HandleRotateCameraValue: MouseDelta is zero. No rotation applied."));
		return;
	}

	AddYawInput(MouseDelta.X * CameraRotationSpeed);
	UE_LOG(LogTemp, Log, TEXT("HandleRotateCameraValue: Added YawInput: %.2f. Controller New ControlRotation: %s"), MouseDelta.X * CameraRotationSpeed, *GetControlRotation().ToString());
}

void AStrategyPlayerController::UpdateCameraMovement(float DeltaTime)
{
	// Placeholder for continuous movement logic like edge scroll
}

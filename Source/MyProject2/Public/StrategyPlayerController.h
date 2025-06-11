// StrategyPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "StrategyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UCurveFloat;
class AStrategyCameraPawn; // Forward declare our new pawn

UCLASS()
class MYPROJECT2_API AStrategyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStrategyPlayerController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	
	// --- Input Actions & Context ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// (Other UInputAction properties remain the same)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> ZoomAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> RotateActionTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> RotateActionValue;

	// --- ADD THESE DEBUGGING VARIABLES ---
	FVector DebugBeforeZoomLocation;
	FVector DebugAfterZoomLocation;
	FTimerHandle DebugAfterZoomTimerHandle;
	void UpdateDebugAfterSphere();
	// --- END OF DEBUGGING VARIABLES ---
	
	// --- Camera Control Parameters ---
	// (All camera control UPROPERTYs remain the same)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float MinZoomLength = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float MaxZoomLength = 5000.0f;

	// ... etc ...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomStepAmount = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Rotation")
	float RotationInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Rotation")
	float CameraRotationSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Movement")
	float CameraMoveSpeed = 1.0f; // Note: This will now be a multiplier for AddMovementInput

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	TObjectPtr<UCurveFloat> CameraPitchByZoomCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ZoomRecenterStrength = 0.5f;


private:
	// A cached pointer to the pawn we are controlling.
	UPROPERTY()
	TObjectPtr<AStrategyCameraPawn> PossessedCameraPawn;
	

	float TargetZoomLength;
	bool bIsRotatingCamera;
	FVector2D LastMousePositionForRotation;

	// --- Input Handling Functions (declarations are the same) ---
	void HandleMoveInput(const FInputActionValue& Value);
	void HandleZoomInput(const FInputActionValue& Value);
	void HandleRotateCameraTrigger(const FInputActionValue& Value);
	void HandleRotateCameraValue(const FInputActionValue& Value);

	void InitializeCameraSettings();
	void UpdateCameraZoomAndPitch(float DeltaTime);
	void UpdateCameraMovement(float DeltaTime);
};
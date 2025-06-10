#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StrategyCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;

UCLASS()
class MYPROJECT2_API AStrategyCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AStrategyCameraPawn();
	virtual void Tick(float DeltaSeconds) override;
	
	// --- Component Getters ---
	// We expose these so the PlayerController can easily access them.
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetViewCamera() const { return ViewCamera; }

protected:
	// The scene component that will act as the pawn's root for movement.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> PawnRootComponent;

	// The spring arm that positions the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	// The camera that views the scene
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> ViewCamera;

	// The component that handles movement logic (e.g., from AddMovementInput)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;
};
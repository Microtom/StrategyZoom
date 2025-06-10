// StrategyGameMode.cpp
#include "StrategyGameMode.h"

#include "StrategyCameraPawn.h"
#include "StrategyPlayerController.h" // Include your player controller

AStrategyGameMode::AStrategyGameMode()
{
	// Set default player controller class
	PlayerControllerClass = AStrategyPlayerController::StaticClass();
	DefaultPawnClass = AStrategyCameraPawn::StaticClass();
	
	// RTS games typically don't have a default pawn class for the player controller to possess
	DefaultPawnClass = nullptr;
}
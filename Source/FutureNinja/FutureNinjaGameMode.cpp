// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FutureNinjaGameMode.h"
#include "FutureNinjaHUD.h"
#include "FutureNinjaCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFutureNinjaGameMode::AFutureNinjaGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	DefaultPawnClass = APawn::StaticClass();

	// use our custom HUD class
	HUDClass = AFutureNinjaHUD::StaticClass();
}

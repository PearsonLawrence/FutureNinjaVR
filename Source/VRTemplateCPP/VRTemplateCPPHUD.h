// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "VRTemplateCPPHUD.generated.h"

class UTexture2D;

UCLASS()
class AVRTemplateCPPHUD : public AHUD
{
	GENERATED_BODY()

public:
	AVRTemplateCPPHUD();

	//Primary draw call for the HUD.
	virtual void DrawHUD() override;

private:
	//Crosshair asset pointer.
	UTexture2D* CrosshairTex;

};


#include "VRTemplateCPP.h"
#include "VRTemplateCPPGameMode.h"
#include "VRTemplateCPPHUD.h"
#include "SmashCharacter.h"

AVRTemplateCPPGameMode::AVRTemplateCPPGameMode() : Super()
{
	//Set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	//Use our custom HUD class
	HUDClass = AVRTemplateCPPHUD::StaticClass();
}

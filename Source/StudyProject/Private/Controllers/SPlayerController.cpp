// SPlayerController.cpp


#include "Controllers/SPlayerController.h"

ASPlayerController::ASPlayerController()
{
}

void ASPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputModeGameOnly;
    SetInputMode(InputModeGameOnly);
}

// SPlayerController.cpp


#include "Controller/SPlayerController.h"
#include "UI/SHUD.h"
#include "Game/SPlayerState.h"
#include "Component/SStatComponent.h"
#include "Character/SCharacter.h"

ASPlayerController::ASPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}
/*
void ASPlayerController::PostInitializeComponents()
{
    UE_LOG(LogTemp, Warning, TEXT("       Start ASPlayerController::PostInitializeComponents()"));
    Super::PostInitializeComponents();
    UE_LOG(LogTemp, Warning, TEXT("       End   ASPlayerController::PostInitializeComponents()"));
}

void ASPlayerController::PlayerTick(float DeltaSeconds)
{
    static bool bOnce = false;
    if (false == bOnce)
    {
        UE_LOG(LogTemp, Warning, TEXT("       Start ASPlayerController::PlayerTick()"));
    }
    Super::PlayerTick(DeltaSeconds);
    if (false == bOnce)
    {
        UE_LOG(LogTemp, Warning, TEXT("       End   ASPlayerController::PlayerTick()"));
        bOnce = true;
    }
}

void ASPlayerController::SetupInputComponent()
{
    UE_LOG(LogTemp, Warning, TEXT("       Start ASPlayerController::SetupInputComponent()"));
    Super::SetupInputComponent();
    UE_LOG(LogTemp, Warning, TEXT("       End   ASPlayerController::SetupInputComponent()"));
}

void ASPlayerController::OnPossess(APawn* aPawn)
{
    UE_LOG(LogTemp, Warning, TEXT("       Start ASPlayerController::OnPossess(ASPlayerPawn)"));
    Super::OnPossess(aPawn);
    UE_LOG(LogTemp, Warning, TEXT("       End   ASPlayerController::OnPossess(ASPlayerPawn)"));
}

void ASPlayerController::OnUnPossess()
{
    UE_LOG(LogTemp, Warning, TEXT("       Start ASPlayerController::OnUnPossess()"));
    Super::OnUnPossess();
    UE_LOG(LogTemp, Warning, TEXT("       End   ASPlayerController::OnUnPossess()"));
}
*/

void ASPlayerController::ToggleInGameMenu()
{
    checkf(IsValid(InGameMenuInstance) == true, TEXT("Invalid InGameMenuInstance"));

    if (false == bIsInGameMenuOn)
    {
        InGameMenuInstance->SetVisibility(ESlateVisibility::Visible);

        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(InGameMenuInstance->GetCachedWidget());
        SetInputMode(Mode);

        // SetPause(true); ���� ���� �Ͻ� ������ ���Ѵٸ�.
        //					�׸��� InputAction �ּ��� TriggerWhenPaused �Ӽ��� true�� �����ؾ� Pause ���¿����� �ش� �Է� �׼��� ������.
        bShowMouseCursor = true;
    }
    else
    {
        InGameMenuInstance->SetVisibility(ESlateVisibility::Collapsed);

        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);

        // SetPause(false); ���� ���� �Ͻ� ������ ���Ѵٸ� �� �ڵ尡 �ʿ���.
        //					�׸��� InputAction �ּ��� TriggerWhenPaused �Ӽ��� true�� �����ؾ� Pause ���¿����� �ش� �Է� �׼��� ������.
        bShowMouseCursor = false;
    }

    bIsInGameMenuOn = !bIsInGameMenuOn;
}

void ASPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputModeGameOnly;
    SetInputMode(InputModeGameOnly);

    if (IsValid(HUDWidgetClass) == true)
    {
        HUDWidget = CreateWidget<USHUD>(this, HUDWidgetClass);
        if (IsValid(HUDWidget) == true)
        {
            HUDWidget->AddToViewport();

            ASPlayerState* SPlayerState = GetPlayerState<ASPlayerState>();
            if (IsValid(SPlayerState) == true)
            {
                HUDWidget->BindPlayerState(SPlayerState);
            }

            ASCharacter* PC = GetPawn<ASCharacter>();
            if (IsValid(PC) == true)
            {
                USStatComponent* StatComponent = PC->GetStatComponent();
                if (IsValid(StatComponent) == true)
                {
                    HUDWidget->BindStatComponent(StatComponent);
                }
            }
        }
    }

    if (IsValid(InGameMenuClass) == true)
    {
        InGameMenuInstance = CreateWidget<UUserWidget>(this, InGameMenuClass);
        if (IsValid(InGameMenuInstance) == true)
        {
            InGameMenuInstance->AddToViewport(3); // ������ ���.

            InGameMenuInstance->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    if (IsValid(CrosshairUIClass) == true)
    {
        UUserWidget* CrosshairUIInstance = CreateWidget<UUserWidget>(this, CrosshairUIClass);
        if (IsValid(CrosshairUIInstance) == true)
        {
            CrosshairUIInstance->AddToViewport(1);

            CrosshairUIInstance->SetVisibility(ESlateVisibility::Visible);
        }
    }
}
/*
void ASPlayerController::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Warning, TEXT("       Start ASPlayerController::EndPlay()"));
    Super::EndPlay(EndPlayReason);
    UE_LOG(LogTemp, Warning, TEXT("       End   ASPlayerController::EndPlay()"));
}
*/
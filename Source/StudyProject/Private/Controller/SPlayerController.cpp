// SPlayerController.cpp


#include "Controller/SPlayerController.h"
#include "UI/SHUD.h"
#include "Game/SPlayerState.h"
#include "Component/SStatComponent.h"
#include "Character/SCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Game/SGameMode.h"
#include "UI/SGameResultWidget.h"
#include "Components/TextBlock.h"

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

        // SetPause(true); 만약 게임 일시 정지를 원한다면.
        //					그리고 InputAction 애셋의 TriggerWhenPaused 속성을 true로 지정해야 Pause 상태에서도 해당 입력 액션이 동작함.
        bShowMouseCursor = true;
    }
    else
    {
        InGameMenuInstance->SetVisibility(ESlateVisibility::Collapsed);

        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);

        // SetPause(false); 만약 게임 일시 정지를 원한다면 이 코드가 필요함.
        //					그리고 InputAction 애셋의 TriggerWhenPaused 속성을 true로 지정해야 Pause 상태에서도 해당 입력 액션이 동작함.
        bShowMouseCursor = false;
    }

    bIsInGameMenuOn = !bIsInGameMenuOn;
}

void ASPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, NotificationText);
}

void ASPlayerController::OnOwningCharacterDead()
{
    ASGameMode* GameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(this));
    if (HasAuthority() == true && IsValid(GameMode) == true)
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("dEATH2")));
        GameMode->OnControllerDead(this);
    }
}

void ASPlayerController::ShowWinnerUI_Implementation()
{
    if (HasAuthority() == false)
    {
        if (IsValid(WinnerUIClass) == true)
        {
            USGameResultWidget* WinnerUI = CreateWidget<USGameResultWidget>(this, WinnerUIClass);
            if (IsValid(WinnerUI) == true)
            {
                WinnerUI->AddToViewport(3);
                WinnerUI->RankingText->SetText(FText::FromString(TEXT("#01")));

                FInputModeUIOnly Mode;
                Mode.SetWidgetToFocus(WinnerUI->GetCachedWidget());
                SetInputMode(Mode);

                bShowMouseCursor = true;
            }
        }
    }
}

void ASPlayerController::ShowLooserUI_Implementation(int32 InRanking)
{
    if (HasAuthority() == false)
    {
        if (IsValid(LooserUIClass) == true)
        {
            USGameResultWidget* LooserUI = CreateWidget<USGameResultWidget>(this, LooserUIClass);
            if (IsValid(LooserUI) == true)
            {
                LooserUI->AddToViewport(3);
                FString RankingString = FString::Printf(TEXT("#%02d"), InRanking);
                LooserUI->RankingText->SetText(FText::FromString(RankingString));

                FInputModeUIOnly Mode;
                Mode.SetWidgetToFocus(LooserUI->GetCachedWidget());
                SetInputMode(Mode);

                bShowMouseCursor = true;
            }
        }
    }
}

void ASPlayerController::ReturnToLobby_Implementation()
{
    if (HasAuthority() == false)
    { // 서버의 레벨이 변경되는걸 원치 않음. 클라가 이동해야하므로 if() 처리 필수.
        UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Loading")), true, FString(TEXT("NextLevel=Lobby?Saved=false")));
    }
}

void ASPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputModeGameOnly;
    SetInputMode(InputModeGameOnly);

    if (HasAuthority() == true)
    {
        return;
    }

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
            InGameMenuInstance->AddToViewport(3); // 상위에 띄움.

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

    if (IsValid(NotificationTextUIClass) == true)
    {
        UUserWidget* NotificationTextUI = CreateWidget<UUserWidget>(this, NotificationTextUIClass);
        if (IsValid(NotificationTextUI) == true)
        {
            NotificationTextUI->AddToViewport(1);

            NotificationTextUI->SetVisibility(ESlateVisibility::Visible);
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
// SGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

class ASPlayerController;

/**
 *
 */
UCLASS()
class STUDYPROJECT_API ASGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASGameMode();

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

    virtual void InitGameState() override;

    virtual void PostInitializeComponents() override;

    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMassage) override;

    virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);

    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual void Logout(AController* Exiting) override;

    virtual void BeginPlay() override;

    void OnControllerDead(ASPlayerController* InDeadController);

private:
    UFUNCTION()
    void OnMainTimerElapsed();

    void NotifyToAllPlayer(const FString& NotificationString);

public:
    FTimerHandle MainTimerHandle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASGameMode")
    int32 WaitingTime = 15;

    int32 RemainWaitingTimeForPlaying = 15;

    int32 MinimumPlayerCountForPlaying = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASGameMode")
    int32 EndingTime = 15;

    int32 RemainWaitingTimeForEnding = 15;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASGameMode", Meta = (AllowPrivateAccess))
    TArray<TObjectPtr<ASPlayerController>> AlivePlayerControllers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASGameMode", Meta = (AllowPrivateAccess))
    TArray<TObjectPtr<ASPlayerController>> DeadPlayerControllers;

};

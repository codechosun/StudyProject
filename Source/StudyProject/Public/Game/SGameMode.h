// SGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

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

};

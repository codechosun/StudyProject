// SPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerController.generated.h"

class USHUD;

/**
 *
 */
UCLASS()
class STUDYPROJECT_API ASPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ASPlayerController();

    USHUD* GetHUDWidget() const { return HUDWidget; };

    void ToggleInGameMenu();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY();
    TObjectPtr<USHUD> HUDWidget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess));
    TSubclassOf<USHUD> HUDWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ASPlayerController, Meta = (AllowPrivateAccess))
    TSubclassOf<UUserWidget> InGameMenuClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ASPlayerController, Meta = (AllowPrivateAccess))
    TObjectPtr<UUserWidget> InGameMenuInstance;

    bool bIsInGameMenuOn = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ASPlayerController, Meta = (AllowPrivateAccess))
    TSubclassOf<UUserWidget> CrosshairUIClass;

};

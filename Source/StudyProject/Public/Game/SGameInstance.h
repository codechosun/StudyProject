// SGameInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h" // SUnrealObject와 다른 이유를 생각해보자. 
#include "SGameInstance.generated.h"

class USPigeon;

/**
 *
 */
UCLASS()
class STUDYPROJECT_API USGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USGameInstance();

	virtual void Init() override;

	virtual void Shutdown() override;

protected:
	UPROPERTY()
	FString Name;

	UPROPERTY()
	TObjectPtr<USPigeon> SpawnedPigeon;

};

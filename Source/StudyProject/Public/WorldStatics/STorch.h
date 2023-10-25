// STorch.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STorch.generated.h"

UCLASS()
class STUDYPROJECT_API ASTorch : public AActor
{
    GENERATED_BODY()

public:
    ASTorch();

private:
    UPROPERTY()
    TObjectPtr<class UBoxComponent> BoxComponent;

    UPROPERTY()
    TObjectPtr<class UStaticMeshComponent> BodyStaticMeshComponent;

    UPROPERTY()
    TObjectPtr<class UPointLightComponent> PointLightComponent;

    UPROPERTY()
    TObjectPtr<class UParticleSystemComponent> ParticleSystemComponent;

};

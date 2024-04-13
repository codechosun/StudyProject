// STorch.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STorch.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UParticleSystemComponent;
class URotatingMovementComponent;

UCLASS()
class STUDYPROJECT_API ASTorch : public AActor
{
    GENERATED_BODY()

public:
    ASTorch();

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ASTorch", Meta=(AllowPrivateAccess))
    TObjectPtr<UBoxComponent> BoxComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASTorch", Meta=(AllowPrivateAccess))
    TObjectPtr<UStaticMeshComponent> BodyStaticMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASTorch", Meta=(AllowPrivateAccess))
    TObjectPtr<UPointLightComponent> PointLightComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASTorch", Meta=(AllowPrivateAccess))
    TObjectPtr<UParticleSystemComponent> ParticleSystemComponent;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "ASTorch", Meta=(AllowPrivateAccess))
    int32 ID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ASTorch", Meta=(AllowPrivateAccess))
    float RotationSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASTorch", meta = (AllowPrivateAccess))
    TObjectPtr<URotatingMovementComponent> RotatingMovementComponent;

};

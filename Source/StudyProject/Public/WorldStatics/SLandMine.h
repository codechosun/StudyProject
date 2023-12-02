// SLandMine.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLandMine.generated.h"

UCLASS()
class STUDYPROJECT_API ASLandMine : public AActor
{
    GENERATED_BODY()

public:
    ASLandMine();

    UFUNCTION()
    void OnLandMineBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION(NetMulticast, Unreliable)
    void SpawnEffect_NetMulticast();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnRep_IsExploded();

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASLandMine", Meta = (AllowPrivateAccess))
    TObjectPtr<class UBoxComponent> BodyBoxComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASLandMine", Meta = (AllowPrivateAccess))
    TObjectPtr<class UStaticMeshComponent> BodyStaticMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASLandMine", meta = (AllowPrivateAccess))
    TObjectPtr<class UParticleSystemComponent> ParticleSystemComponent;

    UPROPERTY(ReplicatedUsing = OnRep_IsExploded, VisibleAnywhere, BlueprintReadOnly, Category = "ASLandMine", meta = (AllowPrivateAccess))
    uint8 bIsExploded : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASLandMine", meta = (AllowPrivateAccess))
    TObjectPtr<class UMaterial> ExplodedMaterial;


};
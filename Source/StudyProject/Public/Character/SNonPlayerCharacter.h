// SNonPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "SCharacter.h"
#include "SNonPlayerCharacter.generated.h"

UCLASS()
class STUDYPROJECT_API ASNonPlayerCharacter : public ASCharacter
{
	GENERATED_BODY()

	friend class UBTTask_Attack;

public:
	ASNonPlayerCharacter();

protected:
	virtual void BeginAttack() override;

	virtual void EndAttack(UAnimMontage* InMontage, bool bInterruped) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> MeleeAttackMontage;
	// 당연한 이야기지만, BP_NPC > Details > MeleeAttackMontange에 AM_NPC_Fire 지정 필수.

};

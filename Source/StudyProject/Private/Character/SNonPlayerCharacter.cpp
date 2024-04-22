// SNonPlayerCharacter.cpp


#include "Character/SNonPlayerCharacter.h"
#include "Controller/SAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/SAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/SPlayerCharacter.h"

ASNonPlayerCharacter::ASNonPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = ASAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// ASNonPlayerCharacter�� ������ ��ġ�ǰų� ���Ӱ� �����Ǹ� SAIController�� ���ǰ� �ڵ����� �����.

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

void ASNonPlayerCharacter::BeginAttack()
{
	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	checkf(IsValid(AnimInstance) == true, TEXT("Invalid AnimInstance"));

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;
	AnimInstance->PlayAnimMontage(MeleeAttackMontage);

	if (OnMeleeAttackMontageEndedDelegate.IsBound() == false)
	{
		OnMeleeAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndAttack);
		AnimInstance->Montage_SetEndDelegate(OnMeleeAttackMontageEndedDelegate, MeleeAttackMontage);
	}
}

void ASNonPlayerCharacter::EndAttack(UAnimMontage* InMontage, bool bInterruped)
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsNowAttacking = false;

	if (OnMeleeAttackMontageEndedDelegate.IsBound() == true)
	{
		OnMeleeAttackMontageEndedDelegate.Unbind();
	}
}

float ASNonPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Super::TakeDamage()���� �������� ó�� �߱� ����.
	if (CurrentHP < KINDA_SMALL_NUMBER)
	{
		ASAIController* AIController = Cast<ASAIController>(GetController());
		if (true == ::IsValid(AIController))
		{
			AIController->EndAI();
		}

		ASPlayerCharacter* DamageCauserCharacter = Cast<ASPlayerCharacter>(DamageCauser);
		if (true == ::IsValid(DamageCauserCharacter))
		{
			DamageCauserCharacter->AddCurrentKillCount(1);
		}

		Destroy();
	}

	return FinalDamageAmount;
}

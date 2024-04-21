// SAnimInstance.cpp


#include "Animation/SAnimInstance.h"
#include "Character/SPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

USAnimInstance::USAnimInstance()
{
}

void USAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CurrentSpeed = 0.f;

	Velocity = FVector::ZeroVector;

	bIsFalling = false;

	bIsCrouching = false;
}

void USAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ASCharacter* OwnerCharacter = Cast<ASCharacter>(TryGetPawnOwner());
	// �� TryGetPawnOwner() �Լ��� Try-�� ������? 
	// TryGetPawnOwner() �Լ��� �ݴ��, ������ �ִ��ν��Ͻ��� ������ ���� GetAnimInstance() �Լ�.
	if (IsValid(OwnerCharacter) == true)
	{
		UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
		if (IsValid(CharacterMovementComponent) == true)
		{
			Velocity = CharacterMovementComponent->GetLastUpdateVelocity();
			CurrentSpeed = Velocity.Size();
			bIsFalling = CharacterMovementComponent->IsFalling();
			bIsCrouching = CharacterMovementComponent->IsCrouching();
		}
	}
}

void USAnimInstance::PlayAnimMontage(UAnimMontage* InAnimMontage)
{
	checkf(IsValid(InAnimMontage) == true, TEXT("Invalid InAnimMontage"));

	if (Montage_IsPlaying(InAnimMontage) == false)
	{
		Montage_Play(InAnimMontage);
	}
}

void USAnimInstance::AnimNotify_CheckHit()
{
	if (OnCheckHit.IsBound() == true)
	{
		// �ش� ��������Ʈ�� 1���� �Լ��� ���ε� �Ǿ� �ִٸ� true�� ��ȯ�ϴ� IsBound() �Լ�
		OnCheckHit.Broadcast();
	}
}

void USAnimInstance::AnimNotify_CheckAttackInput()
{
	if (OnCheckAttackInput.IsBound() == true)
	{
		OnCheckAttackInput.Broadcast();
	}
}

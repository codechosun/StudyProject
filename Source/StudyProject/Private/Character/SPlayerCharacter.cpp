// SPlayerCharacter.cpp


#include "Character/SPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/SInputConfig.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/SWeaponActor.h"
#include "Animation/SAnimInstance.h"

ASPlayerCharacter::ASPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 400.f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
}

void ASPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (IsValid(PlayerController) == true)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (IsValid(Subsystem) == true)
		{
			Subsystem->AddMappingContext(PlayerCharacterInputMappingContext, 0);
		}
	}

	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(AnimInstance) == true)
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnMeleeAttackMontageEnded);
		AnimInstance->OnCheckHit.AddDynamic(this, &ThisClass::OnCheckHit);
		AnimInstance->OnCheckAttackInput.AddDynamic(this, &ThisClass::OnCheckAttackInput);
	}
}

void ASPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetViewMode(EViewMode::TPSView);
}

void ASPlayerCharacter::SetViewMode(EViewMode InViewMode)
{
	if (CurrentViewMode == InViewMode)
	{
		return;
	}

	CurrentViewMode = InViewMode;

	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;

		SpringArmComponent->bUsePawnControlRotation = true;

		SpringArmComponent->bInheritPitch = true;
		SpringArmComponent->bInheritYaw = true;
		SpringArmComponent->bInheritRoll = false;

		SpringArmComponent->bDoCollisionTest = true;

		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;

		break;
	case EViewMode::QuarterView:
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;

		SpringArmComponent->bUsePawnControlRotation = false;

		SpringArmComponent->bInheritPitch = false;
		SpringArmComponent->bInheritYaw = false;
		SpringArmComponent->bInheritRoll = false;

		SpringArmComponent->bDoCollisionTest = false;

		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

		break;
	case EViewMode::TPSView:
	{
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;

		SpringArmComponent->TargetArmLength = 400.f;

		SpringArmComponent->bUsePawnControlRotation = true;

		SpringArmComponent->bInheritPitch = true;
		SpringArmComponent->bInheritYaw = true;
		SpringArmComponent->bInheritRoll = false;

		SpringArmComponent->bDoCollisionTest = true;

		SpringArmComponent->SetRelativeLocation(FVector(0.f, 50.f, 50.f));

		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;

		break;
	}
	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}
}

void ASPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
		break;
	case EViewMode::QuarterView:
	{
		if (KINDA_SMALL_NUMBER < DirectionToMove.SizeSquared())
		{
			GetController()->SetControlRotation(FRotationMatrix::MakeFromX(DirectionToMove).Rotator());
			AddMovementInput(DirectionToMove);
			DirectionToMove = FVector::ZeroVector;
		}
		break;
	}
	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}

	if (KINDA_SMALL_NUMBER < abs(DestArmLength - SpringArmComponent->TargetArmLength))
	{
		SpringArmComponent->TargetArmLength = FMath::FInterpTo(SpringArmComponent->TargetArmLength, DestArmLength, DeltaSeconds, ArmLengthChangeSpeed);
		SpringArmComponent->SetRelativeRotation(FMath::RInterpTo(SpringArmComponent->GetRelativeRotation(), DestArmRotation, DeltaSeconds, ArmRotationChangeSpeed));
	}
}

void ASPlayerCharacter::OnMeleeAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage->GetName().Equals(TEXT("AM_Rifle_Fire_Melee"), ESearchCase::IgnoreCase) == true)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		bIsNowAttacking = false;
	}
}

void ASPlayerCharacter::OnCheckHit()
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CheckHit() has been called.")));
	// 다음 단원에서 Collision에 대해 배움.
}

void ASPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (IsValid(EnhancedInputComponent) == true)
	{
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Move, ETriggerEvent::Triggered, this, &ThisClass::InputMove);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Look, ETriggerEvent::Triggered, this, &ThisClass::InputLook);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->ChangeView, ETriggerEvent::Started, this, &ThisClass::InputChangeView);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->QuickSlot01, ETriggerEvent::Started, this, &ThisClass::InputQuickSlot01);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->QuickSlot02, ETriggerEvent::Started, this, &ThisClass::InputQuickSlot02);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Attack, ETriggerEvent::Started, this, &ThisClass::InputAttack);
	}
}

void ASPlayerCharacter::InputMove(const FInputActionValue& InValue)
{
	FVector2D MovementVector = InValue.Get<FVector2D>();

	switch (CurrentViewMode)
	{
	case EViewMode::TPSView:
	case EViewMode::BackView:
	{ // Switch-Case 구문 내에서 Scope를 지정하면 해당 Scope 내에서 변수 선언이 가능해짐.
		const FRotator ControlRotation = GetController()->GetControlRotation();
		const FRotator ControlRotationYaw(0.f, ControlRotation.Yaw, 0.f);

		const FVector ForwardVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
		const FVector RightVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardVector, MovementVector.X);
		AddMovementInput(RightVector, MovementVector.Y);

		ForwardInputValue = MovementVector.X;
		RightInputValue = MovementVector.Y;

		break;
	}
	case EViewMode::QuarterView:
		DirectionToMove.X = MovementVector.X;
		DirectionToMove.Y = MovementVector.Y;

		break;
	case EViewMode::None:
	case EViewMode::End:
	default:
		AddMovementInput(GetActorForwardVector(), MovementVector.X);
		AddMovementInput(GetActorRightVector(), MovementVector.Y);
		break;
	}
}

void ASPlayerCharacter::InputLook(const FInputActionValue& InValue)
{
	FVector2D LookVector = InValue.Get<FVector2D>();

	switch (CurrentViewMode)
	{
	case EViewMode::TPSView:
	case EViewMode::BackView:
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
		break;
	case EViewMode::QuarterView:
	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}
}

void ASPlayerCharacter::InputChangeView(const FInputActionValue& InValue)
{
	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
		/* Case 1. BackView에서 QuarterView로

		BackView는 컨트롤 로테이션이 스프링암 로테이션에 동기화 되고 있음.
		QuarterView는 컨트롤 로테이션에 폰 로테이션이 동기화 됨.
		따라서 시점 변경 전에 컨트롤 로테이션에 폰의 로테이션을 세팅해둬야 함.

		*/
		GetController()->SetControlRotation(GetActorRotation());
		DestArmLength = 800.f;
		DestArmRotation = FRotator(-45.f, 0.f, 0.f);
		SetViewMode(EViewMode::QuarterView);
		
		break;
	case EViewMode::QuarterView:
		/* Case 2. QuarterView에서 BackView로

		QuarterView는 컨트롤 로테이션에 폰 로테이션이 동기화 됨.
		BackView는 컨트롤 로테이션이 스프링암 로테이션에 동기화 되고 있음.
		따라서 시점 변경 전에 컨트롤 로테이션에 스프링암 로테이션을 세팅해둬야 함.
		ss
		*/
		GetController()->SetControlRotation(SpringArmComponent->GetRelativeRotation());
		DestArmLength = 400.f;
		DestArmRotation = FRotator::ZeroRotator;
		SetViewMode(EViewMode::BackView);

		break;
	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}
}

void ASPlayerCharacter::InputQuickSlot01(const FInputActionValue& InValue)
{
	FName WeaponSocket(TEXT("WeaponSocket"));
	if (GetMesh()->DoesSocketExist(WeaponSocket) == true && IsValid(WeaponInstance) == false)
	{
		WeaponInstance = GetWorld()->SpawnActor<ASWeaponActor>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (IsValid(WeaponInstance) == true)
		{
			WeaponInstance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
		}
	}
}

void ASPlayerCharacter::InputQuickSlot02(const FInputActionValue& InValue)
{
	if (IsValid(WeaponInstance) == true)
	{
		WeaponInstance->Destroy();
		WeaponInstance = nullptr;
	}
}

void ASPlayerCharacter::InputAttack(const FInputActionValue& InValue)
{
	if (GetCharacterMovement()->IsFalling() == true)
	{
		return;
	}

	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	checkf(IsValid(AnimInstance) == true, TEXT("Invalid AnimInstance"));

	if (IsValid(WeaponInstance) == true)
	{
		if (IsValid(WeaponInstance->GetMeleeAttackMontage()) == true)
		{
			if (0 == CurrentComboCount)
			{
				BeginCombo();
			}
			else
			{
				ensure(FMath::IsWithinInclusive<int32>(CurrentComboCount, 1, MaxComboCount));
				bIsAttackKeyPressed = true;
			}
		}
	}
}

void ASPlayerCharacter::BeginCombo()
{
	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	checkf(IsValid(AnimInstance) == true, TEXT("Invalid AnimInstance"));
	checkf(IsValid(WeaponInstance) == true, TEXT("Invalid WeaponInstance"));

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;
	AnimInstance->PlayAnimMontage(WeaponInstance->GetMeleeAttackMontage());

	CurrentComboCount = 1;

	if (OnMeleeAttackMontageEndedDelegate.IsBound() == false)
	{
		OnMeleeAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndCombo);
		AnimInstance->Montage_SetEndDelegate(OnMeleeAttackMontageEndedDelegate, WeaponInstance->GetMeleeAttackMontage());
	}
}

void ASPlayerCharacter::OnCheckAttackInput()
{
	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	checkf(IsValid(AnimInstance) == true, TEXT("Invalid AnimInstance"));
	checkf(IsValid(WeaponInstance) == true, TEXT("Invalid WeaponInstance"));

	if (bIsAttackKeyPressed == true)
	{
		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, MaxComboCount);

		FName NextSectionName = *FString::Printf(TEXT("%s%d"), *AttackAnimMontageSectionName, CurrentComboCount);
		AnimInstance->Montage_JumpToSection(NextSectionName, WeaponInstance->GetMeleeAttackMontage());
		bIsAttackKeyPressed = false;
	}
}

void ASPlayerCharacter::EndCombo(UAnimMontage* InMontage, bool bInterruped)
{
	checkf(IsValid(WeaponInstance) == true, TEXT("Invalid WeaponInstance"));

	ensureMsgf(CurrentComboCount != 0, TEXT("CurrentComboCount == 0"));
	CurrentComboCount = 0;
	bIsAttackKeyPressed = false;
	bIsNowAttacking = false;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	if (OnMeleeAttackMontageEndedDelegate.IsBound() == true)
	{
		OnMeleeAttackMontageEndedDelegate.Unbind();
	}
}

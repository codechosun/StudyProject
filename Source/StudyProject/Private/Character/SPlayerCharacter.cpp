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
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "Particles/ParticleSystemComponent.h"
#include "Component/SStatComponent.h"
#include "SPlayerCharacterSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Controller/SPlayerController.h"
#include "WorldStatic/SLandMine.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ASPlayerCharacter::ASPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 400.f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(GetRootComponent());
	ParticleSystemComponent->SetAutoActivate(false);

	const USPlayerCharacterSettings* CDO = GetDefault<USPlayerCharacterSettings>();
	if (0 < CDO->PlayerCharacterMeshMaterialPaths.Num())
	{
		for (FSoftObjectPath PlayerCharacterMeshPath : CDO->PlayerCharacterMeshMaterialPaths)
		{
			UE_LOG(LogTemp, Warning, TEXT("Path: %s"), *(PlayerCharacterMeshPath.ToString()));
		}
	}

	TimeBetweenFire = 60.f / FirePerMinute;
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

	SetViewMode(EViewMode::TPSView);
}

void ASPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
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

	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaSeconds, 35.f);
	CameraComponent->SetFieldOfView(CurrentFOV);

	if (IsValid(GetController()) == true)
	{
		PreviousAimPitch = CurrentAimPitch;
		PreviousAimYaw = CurrentAimYaw;

		FRotator ControlRotation = GetController()->GetControlRotation();
		CurrentAimPitch = ControlRotation.Pitch;
		CurrentAimYaw = ControlRotation.Yaw;
	}

	if (PreviousAimPitch != CurrentAimPitch || PreviousAimYaw != CurrentAimYaw)
	{
		if (false == HasAuthority())
		{
			UpdateAimValue_Server(CurrentAimPitch, CurrentAimYaw);
		}
	}

	if (PreviousForwardInputValue != ForwardInputValue || PreviousRightInputValue != RightInputValue)
	{
		if (false == HasAuthority())
		{
			UpdateInputValue_Server(ForwardInputValue, RightInputValue);
		}
	}

	if (true == bIsNowRagdollBlending)
	{
		CurrentRagDollBlendWeight = FMath::FInterpTo(CurrentRagDollBlendWeight, TargetRagDollBlendWeight, DeltaSeconds, 10.f);

		FName PivotBoneName = FName(TEXT("spine_01"));
		GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(PivotBoneName, CurrentRagDollBlendWeight);

		if (CurrentRagDollBlendWeight - TargetRagDollBlendWeight < KINDA_SMALL_NUMBER)
		{
			GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, false);
			bIsNowRagdollBlending = false;
		}

		if (true == ::IsValid(GetStatComponent()) && GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
		{
			GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(FName(TEXT("root")), 1.f);
			// 모든 본에 렉돌 가중치
			GetMesh()->SetSimulatePhysics(true);
			bIsNowRagdollBlending = false;
		}
	}

	return;

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

void ASPlayerCharacter::UpdateInputValue_Server_Implementation(const float& InForwardInputValue,
	const float& InRightInputValue)
{
	ForwardInputValue = InForwardInputValue;
	RightInputValue = InRightInputValue;
}

void ASPlayerCharacter::UpdateAimValue_Server_Implementation(const float& InAimPitchValue, const float& InAimYawValue)
{
	CurrentAimPitch = InAimPitchValue;
	CurrentAimYaw = InAimYawValue;
}

void ASPlayerCharacter::SetMeshMaterial(const EPlayerTeam& InPlayerTeam)
{
	uint8 TeamIdx = 0u;
	switch (InPlayerTeam)
	{
	case EPlayerTeam::Black:
		TeamIdx = 0u;
		break;
	case EPlayerTeam::White:
		TeamIdx = 1u;
		break;
	default:
		break;
	}

	const USPlayerCharacterSettings* CDO = GetDefault<USPlayerCharacterSettings>();
	CurrentPlayerCharacterMeshMaterialPath = CDO->PlayerCharacterMeshMaterialPaths[TeamIdx];
	AssetStreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		CurrentPlayerCharacterMeshMaterialPath,
		FStreamableDelegate::CreateLambda([this]() -> void
			{
				AssetStreamableHandle->ReleaseHandle();
				TSoftObjectPtr<UMaterial> LoadedMaterialInstanceAsset(CurrentPlayerCharacterMeshMaterialPath);
				if (LoadedMaterialInstanceAsset.IsValid() == true)
				{
					GetMesh()->SetMaterial(0, LoadedMaterialInstanceAsset.Get());
				}
			})
	);
}

float ASPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	PlayRagdoll_NetMulticast();

	return FinalDamage;
}

void ASPlayerCharacter::PlayRagdoll_NetMulticast_Implementation()
{
	if (IsValid(GetStatComponent()) == false)
	{
		return;
	}

	if (GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
	{
		GetMesh()->SetSimulatePhysics(true);
	}
	else
	{
		FName PivotBoneName = FName(TEXT("spine_01"));
		GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, true);
		TargetRagDollBlendWeight = 1.f;
		HittedRagdollRestoreTimerDelegate.BindUObject(this, &ThisClass::OnHittedRagdollRestoreTimerElapsed);
		GetWorld()->GetTimerManager().SetTimer(HittedRagdollRestoreTimer, HittedRagdollRestoreTimerDelegate, 1.f, false);
	}
}

void ASPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ForwardInputValue);
	DOREPLIFETIME(ThisClass, RightInputValue);
	DOREPLIFETIME(ThisClass, CurrentAimPitch);
	DOREPLIFETIME(ThisClass, CurrentAimYaw);
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
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Menu, ETriggerEvent::Started, this, &ThisClass::InputMenu);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->IronSight, ETriggerEvent::Started, this, &ThisClass::StartIronSight);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->IronSight, ETriggerEvent::Completed, this, &ThisClass::EndIronSight);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Trigger, ETriggerEvent::Started, this, &ThisClass::ToggleTrigger);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Attack, ETriggerEvent::Started, this, &ThisClass::StartFire);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Attack, ETriggerEvent::Completed, this, &ThisClass::StopFire);
	}
}

void ASPlayerCharacter::InputMove(const FInputActionValue& InValue)
{
	if (GetCharacterMovement()->GetGroundMovementMode() == MOVE_None || StatComponent->GetCurrentHP() <= KINDA_SMALL_NUMBER)
	{
		return;
	}

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
	if (GetCharacterMovement()->GetGroundMovementMode() == MOVE_None || StatComponent->GetCurrentHP() <= KINDA_SMALL_NUMBER)
	{
		return;
	}

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
	SpawnWeaponInstance_Server();
}

void ASPlayerCharacter::InputQuickSlot02(const FInputActionValue& InValue)
{
	if (IsValid(WeaponInstance) == true)
	{
		DestroyWeaponInstance_Server();
	}
}

void ASPlayerCharacter::SpawnWeaponInstance_Server_Implementation()
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

void ASPlayerCharacter::DestroyWeaponInstance_Server_Implementation()
{
	if (IsValid(WeaponInstance) == true)
	{
		WeaponInstance->Destroy();
		WeaponInstance = nullptr;
	}
}

void ASPlayerCharacter::OnRep_WeaponInstance()
{
	if (IsValid(WeaponInstance) == true)
	{
		TSubclassOf<UAnimInstance> RifleCharacterAnimLayer = WeaponInstance->GetArmedCharacterAnimLayer();
		if (IsValid(RifleCharacterAnimLayer) == true)
		{
			GetMesh()->LinkAnimClassLayers(RifleCharacterAnimLayer);
		}

		USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
		if (IsValid(AnimInstance) == true && IsValid(WeaponInstance->GetEquipAnimMontage()))
		{
			AnimInstance->Montage_Play(WeaponInstance->GetEquipAnimMontage());
		}

		UnarmedCharacterAnimLayer = WeaponInstance->GetUnarmedCharacterAnimLayer();
		UnequipAnimMontage = WeaponInstance->GetUnequipAnimMontage();
	}
	else
	{
		if (IsValid(UnarmedCharacterAnimLayer) == true)
		{
			GetMesh()->LinkAnimClassLayers(UnarmedCharacterAnimLayer);
		}

		USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
		if (IsValid(UnequipAnimMontage) == true)
		{
			AnimInstance->Montage_Play(UnequipAnimMontage);
		}
	}
}

void ASPlayerCharacter::InputAttack(const FInputActionValue& InValue)
{
	if (GetCharacterMovement()->IsFalling() == true)
	{
		return;
	}

	if (bIsTriggerToggle == false)
	{
		TryFire();
	}
}

void ASPlayerCharacter::InputMenu(const FInputActionValue& InValue)
{
	ASPlayerController* PlayerController = GetController<ASPlayerController>();
	if (true == ::IsValid(PlayerController))
	{
		PlayerController->ToggleInGameMenu();
	}
}

void ASPlayerCharacter::TryFire()
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (IsValid(PlayerController) == true && IsValid(WeaponInstance) == true)
	{
#pragma region CaculateTargetTransform
		float FocalDistance = 400.f;
		FVector FocalLocation;
		FVector CameraLocation;
		FRotator CameraRotation;

		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();
		FocalLocation = CameraLocation + (AimDirectionFromCamera * FocalDistance);

		FVector WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
		FVector FinalFocalLocation = FocalLocation + (((WeaponMuzzleLocation - FocalLocation) | AimDirectionFromCamera) * AimDirectionFromCamera);

		FTransform TargetTransform = FTransform(CameraRotation, FinalFocalLocation);

		if (ShowAttackDebug == 1)
		{
			DrawDebugSphere(GetWorld(), WeaponMuzzleLocation, 2.f, 16, FColor::Red, false, 180.f);

			DrawDebugSphere(GetWorld(), CameraLocation, 2.f, 16, FColor::Yellow, false, 180.f);

			DrawDebugSphere(GetWorld(), FinalFocalLocation, 2.f, 16, FColor::Magenta, false, 180.f);

			// (WeaponLoc - FocalLoc)
			DrawDebugLine(GetWorld(), FocalLocation, WeaponMuzzleLocation, FColor::Yellow, false, 180.f, 0, 2.f);

			// AimDir
			DrawDebugLine(GetWorld(), CameraLocation, FinalFocalLocation, FColor::Blue, false, 180.f, 0, 2.f);

			// Project Direction Line
			DrawDebugLine(GetWorld(), WeaponMuzzleLocation, FinalFocalLocation, FColor::Red, false, 180.f, 0, 2.f);
		}

#pragma endregion

#pragma region PerformLineTracing

		FVector BulletDirection = TargetTransform.GetUnitAxis(EAxis::X);
		FVector StartLocation = TargetTransform.GetLocation();
		FVector EndLocation = StartLocation + BulletDirection * WeaponInstance->GetMaxRange();

		FHitResult HitResult;
		FCollisionQueryParams TraceParams(NAME_None, false, this);
		TraceParams.AddIgnoredActor(WeaponInstance);

		bool IsCollided = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel2, TraceParams);
		if (IsCollided == false)
		{
			HitResult.TraceStart = StartLocation;
			HitResult.TraceEnd = EndLocation;
		}

		if (ShowAttackDebug == 2)
		{
			if (IsCollided == true)
			{
				DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);

				DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 2.f, 16, FColor::Green, false, 60.f);

				DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Blue, false, 60.f, 0, 2.f);
			}
			else
			{
				DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);

				DrawDebugSphere(GetWorld(), EndLocation, 2.f, 16, FColor::Green, false, 60.f);

				DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, 60.f, 0, 2.f);
			}
		}

#pragma endregion

		ApplyDamageAndDrawLine_Server(HitResult);

		// 1. Owning Client에서도 몽타주 재생
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance) == true && IsValid(WeaponInstance) == true)
		{
			if (AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()) == false)
			{
				AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
			}
		}

		// 2. Other Client에서도 재생하기 위해 Server RPC 호출.
		//    이전의 무기 장착/탈거 몽타주 재생 로직과 이번 격발 몽타주 재생에서 로직 차이는 뭘까?
		PlayAttackMontage_Server();
	}

	if (IsValid(FireShake) == true && GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->ClientStartCameraShake(FireShake);
	}

}

void ASPlayerCharacter::ApplyDamageAndDrawLine_Server_Implementation(FHitResult HitResult)
{
	ASCharacter* HittedCharacter = Cast<ASCharacter>(HitResult.GetActor());
	if (IsValid(HittedCharacter) == true)
	{
		FDamageEvent DamageEvent;

		FString BoneNameString = HitResult.BoneName.ToString();

		if (true == BoneNameString.Equals(FString(TEXT("HEAD")), ESearchCase::IgnoreCase))
		{
			HittedCharacter->TakeDamage(100.f, DamageEvent, GetController(), this);
		}
		else
		{
			HittedCharacter->TakeDamage(10.f, DamageEvent, GetController(), this);
		}
	}

	DrawLine_NetMulticast(HitResult.TraceStart, HitResult.TraceEnd);
}

void ASPlayerCharacter::DrawLine_NetMulticast_Implementation(const FVector& InDrawStart, const FVector& InDrawEnd)
{
	if (HasAuthority() == false)
	{
		DrawDebugLine(GetWorld(), WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash")), InDrawEnd, FColor(255, 255, 255, 64), false, 0.1f, 0U, 0.5f);
	}
}

void ASPlayerCharacter::PlayAttackMontage_Server_Implementation()
{
	PlayAttackMontage_NetMulticast();
}

void ASPlayerCharacter::PlayAttackMontage_NetMulticast_Implementation()
{
	if (HasAuthority() == false && GetOwner() != UGameplayStatics::GetPlayerController(this, 0))
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance) == true && IsValid(WeaponInstance) == true)
		{
			if (AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()) == false)
			{
				AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
			}
		}
	}
}

void ASPlayerCharacter::StartIronSight(const FInputActionValue& InValue)
{
	TargetFOV = 45.f;
}

void ASPlayerCharacter::EndIronSight(const FInputActionValue& InValue)
{
	TargetFOV = 70.f;
}

void ASPlayerCharacter::ToggleTrigger(const FInputActionValue& InValue)
{
	bIsTriggerToggle = !bIsTriggerToggle;
}

void ASPlayerCharacter::StartFire(const FInputActionValue& InValue)
{
	if (bIsTriggerToggle == true)
	{
		GetWorldTimerManager().SetTimer(BetweenShotsTimer, this, &ThisClass::TryFire, TimeBetweenFire, true);
	}
}

void ASPlayerCharacter::StopFire(const FInputActionValue& InValue)
{
	GetWorldTimerManager().ClearTimer(BetweenShotsTimer);
}

void ASPlayerCharacter::OnHittedRagdollRestoreTimerElapsed()
{
	FName PivotBoneName = FName(TEXT("spine_01"));
	TargetRagDollBlendWeight = 0.f;
	CurrentRagDollBlendWeight = 1.f;
	bIsNowRagdollBlending = true;
}

void ASPlayerCharacter::SpawnLandMine(const FInputActionValue& InValue)
{
	SpawnLandMine_Server();
}

bool ASPlayerCharacter::SpawnLandMine_Server_Validate()
{
	return true;
}

void ASPlayerCharacter::SpawnLandMine_Server_Implementation()
{
	if (true == ::IsValid(LandMineClass))
	{
		FVector SpawnedLocation = (GetActorLocation() + GetActorForwardVector() * 200.f) - FVector(0.f, 0.f, 90.f);
		ASLandMine* SpawnedLandMine = GetWorld()->SpawnActor<ASLandMine>(LandMineClass, SpawnedLocation, FRotator::ZeroRotator);
		SpawnedLandMine->SetOwner(GetController());
	}
}

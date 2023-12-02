// STPSCharacter.cpp


#include "Characters/STPSCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/SInputConfigData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Controllers/SPlayerController.h"
#include "Components/SStatComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "WorldStatics/SLandMine.h"

ASTPSCharacter::ASTPSCharacter()
    : ASCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("SCharacter"));

    CameraComponent->SetRelativeLocation(FVector(0.f, 60.f, 20.f)); // TPS 방식의 슈팅 게임 특징.

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->bDoCollisionTest = true;
    SpringArmComponent->bInheritPitch = true;
    SpringArmComponent->bInheritYaw = true;
    SpringArmComponent->bInheritRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);

    WeaponSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMeshComponent"));

    TimeBetweenFire = 60.f / FirePerMinute;
}

void ASTPSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaSeconds, 35.f);
    CameraComponent->SetFieldOfView(CurrentFOV);

    if (true == ::IsValid(GetController()))
    {
        FRotator ControlRotation = GetController()->GetControlRotation();
        CurrentAimPitch = ControlRotation.Pitch;
        CurrentAimYaw = ControlRotation.Yaw;
    }
}

void ASTPSCharacter::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (true == ::IsValid(PlayerController))
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
        if (true == ::IsValid(Subsystem))
        {
            Subsystem->AddMappingContext(PlayerCharacterInputMappingContext, 0);
        }
    }

    FName EquipmentSocketName = FName(TEXT("EquipmentSocket"));
    if (true == GetMesh()->DoesSocketExist(EquipmentSocketName))
    {
        WeaponSkeletalMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, EquipmentSocketName);
    }
}

float ASTPSCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    if (false == ::IsValid(GetStatComponent()))
    {
        return ActualDamage;
    }

    if (GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
    {
        GetMesh()->SetSimulatePhysics(true);
    }

    return ActualDamage;
}

void ASTPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (true == ::IsValid(EnhancedInputComponent))
    {
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->IronSightAction, ETriggerEvent::Started, this, &ThisClass::StartIronSight);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->IronSightAction, ETriggerEvent::Completed, this, &ThisClass::EndIronSight);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->TriggerAction, ETriggerEvent::Started, this, &ThisClass::ToggleTrigger);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->AttackAction, ETriggerEvent::Started, this, &ThisClass::StartFire);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->AttackAction, ETriggerEvent::Completed, this, &ThisClass::StopFire);
        EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LandMineAction, ETriggerEvent::Started, this, &ThisClass::SpawnLandMine);
    }
}

void ASTPSCharacter::Move(const FInputActionValue& InValue)
{
    FVector2D MovementVector = InValue.Get<FVector2D>();
    ForwardInputValue = MovementVector.X;
    RightInputValue = MovementVector.Y;

    const FRotator CurrentControlRotation = GetController()->GetControlRotation();
    const FRotator CurrentControlRotationYaw(0.f, CurrentControlRotation.Yaw, 0.f);

    FVector ForwardDirection = FRotationMatrix(CurrentControlRotationYaw).GetUnitAxis(EAxis::X);
    FVector RightDirection = FRotationMatrix(CurrentControlRotationYaw).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, MovementVector.X);
    AddMovementInput(RightDirection, MovementVector.Y);
}

void ASTPSCharacter::Look(const FInputActionValue& InValue)
{
    FVector2D LookAxisVector = InValue.Get<FVector2D>();

    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void ASTPSCharacter::Attack(const FInputActionValue& InValue)
{
    if (false == bIsTriggerToggle)
    {
        Fire();
    }

    ASPlayerController* PlayerController = Cast<ASPlayerController>(GetController());
    if (true == ::IsValid(PlayerController) && true == ::IsValid(FireShake))
    {
        PlayerController->ClientStartCameraShake(FireShake);
    }
}

void ASTPSCharacter::Fire()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (false == ::IsValid(PlayerController))
    {
        return;
    }

    //FHitResult HitResult;
    FHitResult HitResult1; // CameraStart to CameraEnd
    FHitResult HitResult2; // Muzzle to CameraEnd

    FVector CameraStartLocation = CameraComponent->GetComponentLocation();
    FVector CameraEndLocation = CameraStartLocation + CameraComponent->GetForwardVector() * 5000.f;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredComponent((const UPrimitiveComponent*)(CameraComponent));
    QueryParams.bTraceComplex = true;
    // 충돌 검지 시에 좀 더 복잡한 모양의 충돌체를 기준으로 검지할지에 대한 속성.
    // Content Browser > StarterContent > Props > SM_Chair 더블클릭.
    // Details > Collision Complexity를 Use Complex Collision As Simple로 설정하면
    // 쿼리(충돌 검지) 요청시 복잡한 모양에 대한 쿼리를 제공함.
    // 충돌 계산 부하는 증가하지만 그만큼 현실적인 게임 플레이 가능.
    // SM_Chair > Toolbar > Collision > Auto Convex Collision 클릭 후 우하단 Convex Decomposition으로
    // 복잡한 모양의 충돌체를 손쉽게 제작 가능.

    bool bIsCollide = GetWorld()->LineTraceSingleByChannel(HitResult1, CameraStartLocation, CameraEndLocation, ECC_Visibility, QueryParams);

    FVector MuzzleLocation = WeaponSkeletalMeshComponent->GetSocketLocation(FName("MuzzleSocket"));
    if (true == bIsCollide)
    {
        DrawDebugLine(GetWorld(), MuzzleLocation, HitResult1.Location, FColor(255, 255, 255, 64), true, 0.1f, 0U, 0.5f);

        bIsCollide = GetWorld()->LineTraceSingleByChannel(HitResult2, MuzzleLocation, HitResult1.Location, ECC_GameTraceChannel2, QueryParams);
        if (true == bIsCollide)
        {
            FDamageEvent DamageEvent;
            HitResult2.GetActor()->TakeDamage(50.f, DamageEvent, GetController(), this);
        }
    }
    else
    {
        DrawDebugLine(GetWorld(), MuzzleLocation, CameraEndLocation, FColor(255, 255, 255, 64), false, 0.1f, 0U, 0.5f);
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (false == ::IsValid(AnimInstance))
    {
        return;
    }

    if (false == AnimInstance->Montage_IsPlaying(RifleFireAnimMontage))
    {
        AnimInstance->Montage_Play(RifleFireAnimMontage);
    }
}

void ASTPSCharacter::StartIronSight(const FInputActionValue& InValue)
{
    TargetFOV = 45.f;
}

void ASTPSCharacter::EndIronSight(const FInputActionValue& InValue)
{
    TargetFOV = 70.f;
}

void ASTPSCharacter::ToggleTrigger(const FInputActionValue& InValue)
{
    bIsTriggerToggle = !bIsTriggerToggle;
}

void ASTPSCharacter::StartFire(const FInputActionValue& InValue)
{
    if (true == bIsTriggerToggle)
    {
        GetWorldTimerManager().SetTimer(BetweenShotsTimer, this, &ThisClass::Fire, TimeBetweenFire, true);
    }
}

void ASTPSCharacter::StopFire(const FInputActionValue& InValue)
{
    GetWorldTimerManager().ClearTimer(BetweenShotsTimer);
}

void ASTPSCharacter::SpawnLandMine(const FInputActionValue& InValue)
{
    SpawnLandMine_Server();
}

bool ASTPSCharacter::SpawnLandMine_Server_Validate()
{
    return true;
}

void ASTPSCharacter::SpawnLandMine_Server_Implementation()
{
    if (true == ::IsValid(LandMineClass))
    {
        FVector SpawnedLocation = (GetActorLocation() + GetActorForwardVector() * 200.f) - FVector(0.f, 0.f, 90.f);
        ASLandMine* SpawnedLandMine = GetWorld()->SpawnActor<ASLandMine>(LandMineClass, SpawnedLocation, FRotator::ZeroRotator);
        SpawnedLandMine->SetOwner(GetController());
    }
}

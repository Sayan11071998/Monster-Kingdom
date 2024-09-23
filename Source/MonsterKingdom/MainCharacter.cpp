#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon.h"
#include "Enemy.h"

AMainCharacter::AMainCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	// Add Character Movements
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 550.f;
	GetCharacterMovement()->AirControl = 0.2f;

}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetMovementSpeedAndAir();

}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::SprintKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::SprintKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMainCharacter::LMBDown);

}

void AMainCharacter::MoveForward(float Value)
{
	if(!bIsAlive)
		return;
	
	if(Value != 0.f && !bIsAttacking)
	{
		auto YawRotation = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
		auto Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{
	if(!bIsAlive)
		return;

	if(Value != 0.f && !bIsAttacking)
	{
		auto YawRotation = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
		auto Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::Jump()
{
	if(!bIsAlive)
		return;

	if(bIsAttacking)
		return;

	Super::Jump();
}

void AMainCharacter::SprintKeyDown()
{
	if(!bIsAlive)
		return;

	if(GetCharacterMovement()->Velocity.Size() == 0)
		return;
	
	MovementState = EMovementState::MS_Sprinting;
	GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
}

void AMainCharacter::SprintKeyUp()
{
	if(!bIsAlive)
		return;

	MovementState = EMovementState::MS_Normal;
	GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
}

void AMainCharacter::LMBDown()
{
	if(!bIsAlive)
		return;

	if(EquippedWeapon)
	{
		UAnimInstance* MyAnim = GetMesh()->GetAnimInstance();

		if(MyAnim && CombatMontage)
		{
			MyAnim->Montage_Play(CombatMontage, 2.5f);
			float RandAnim = FMath::RandRange(0, 1);
			if(RandAnim > 0)
			{
				MyAnim->Montage_JumpToSection(TEXT("Attack_1"));
			}
			else
			{
				MyAnim->Montage_JumpToSection(TEXT("Attack_2"));
			}
			bCanDetectDamageCollision = true;
		}
	}
}

void AMainCharacter::SetMovementSpeedAndAir()
{
	auto CharSpeed = GetVelocity();
	auto LateralSpeed = FVector(CharSpeed.X, CharSpeed.Y, 0.f);

	MovementSpeed = LateralSpeed.Size();

	bInAir = GetMovementComponent()->IsFalling();
}

void AMainCharacter::EquipWeapon(AWeapon* WeaponActor)
{
	if(EquippedWeapon != nullptr)
		return;
	
	WeaponActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("RightHandSocket"));
	WeaponActor->Used = true;
	EquippedWeapon = WeaponActor;

	// Add On Component Begin Overlap For Hit Box
	EquippedWeapon->AttackHitBox->OnComponentBeginOverlap.AddDynamic(this, &AMainCharacter::DamageBoxOnBeginOverlap);

}

void AMainCharacter::DamageBoxOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bSweepFrom, const FHitResult& SweepResults)
{
	if(bCanDetectDamageCollision)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if(Enemy)
		{
			bCanDetectDamageCollision = false;
			Enemy->ApplyDamage();
		}
	}
}

void AMainCharacter::ApplyDamage()
{
	if(!bIsAlive)
		return;

	Health -= 10;

	if(Health <= 0.f)
	{
		bIsAlive = false;

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AMainCharacter::RestartGame, 3.f);
	}
}

void AMainCharacter::RestartGame()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}
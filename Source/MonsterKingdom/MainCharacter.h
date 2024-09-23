#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AWeapon;
class UAnimMontage;

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	MS_Normal UMETA(DisplayName = "Normal"),
	MS_Sprinting UMETA(DisplayName = "Sprinting"),
};

UCLASS()
class MONSTERKINGDOM_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMainCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UCameraComponent* FollowCamera;

	void MoveForward(float Value);
	void MoveRight(float Value);

	virtual void Jump() override;
	
	void SprintKeyDown();
	void SprintKeyUp();

	void LMBDown();
	
	UPROPERTY(BlueprintReadOnly)
		bool bIsAttacking;
	UPROPERTY(BlueprintReadOnly)
		bool bIsAlive = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EMovementState MovementState;

	UPROPERTY(BlueprintReadOnly)
		AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly)
		float MovementSpeed;

	UPROPERTY(BlueprintReadOnly)
		bool bInAir;

	void SetMovementSpeedAndAir();

	float RunningSpeed = 600.f;
	float SprintingSpeed = 950.f;

	void EquipWeapon(AWeapon* WeaponActor);

	UPROPERTY(EditAnywhere)
		UAnimMontage* CombatMontage;

	UFUNCTION()
		virtual void DamageBoxOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bSweepFrom, const FHitResult& SweepResults);

	UPROPERTY(BlueprintReadWrite)
		bool bCanDetectDamageCollision;

	UPROPERTY(BlueprintReadWrite)
		float Health = 100.f;

	void ApplyDamage();
	void RestartGame();

};
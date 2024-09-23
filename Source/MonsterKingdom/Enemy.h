#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

class USphereComponent;
class UBoxComponent;
class AMainCharacter;
class UAnimMontage;
class AAIController;

UCLASS()
class MONSTERKINGDOM_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
		USphereComponent* DetectSphere;
	UPROPERTY(EditAnywhere)
		USphereComponent* CombatSphere;
	UPROPERTY(EditAnywhere)
		UBoxComponent* AttackHitBox;
	
	UFUNCTION()
		virtual void DetectSphereOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bSweepFrom, const FHitResult& SweepResults);

	UFUNCTION()
		virtual void DetectSphereOnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		virtual void CombatSphereOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bSweepFrom, const FHitResult& SweepResults);

	UFUNCTION()
		virtual void CombatSphereOnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		virtual void AttackBoxOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bSweepFrom, const FHitResult& SweepResults);

	AMainCharacter* TargetChar;
	AAIController* AI_Controller;
	void MoveToTarget();

	UPROPERTY(BlueprintReadOnly)
		bool bIsAlive = true;

	bool bAttacking;

	UPROPERTY(EditAnywhere)
		float MoveToTargetRadius = 10.f;

	UPROPERTY(EditAnywhere)
		bool bTargetInAttackRange;
	
	float AttackDelayMin = 0.2f;
	float AttackDelayMax = 0.7f;

	bool CanDetectDamageCollision;

	void Attack();

	UFUNCTION(BlueprintCallable)
		void AttackEnded();

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere)
		UAnimMontage* CombatMontage;

	float Health = 100;

	void ApplyDamage();
	void DisposeEnemy();

};

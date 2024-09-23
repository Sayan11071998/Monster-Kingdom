#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIModule/Classes/AIController.h"
#include "MainCharacter.h"
#include "Animation/AnimInstance.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	DetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Detect Sphere"));
	DetectSphere->SetupAttachment(GetRootComponent());
	DetectSphere->InitSphereRadius(600.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Combat Sphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	AttackHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitBox"));
	AttackHitBox->SetupAttachment(GetMesh(), TEXT("HitboxSocket"));
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AI_Controller = Cast<AAIController>(GetController());

	DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::DetectSphereOnBeginOverlap);
	DetectSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::DetectSphereOnEndOverlap);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnBeginOverlap);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnEndOverlap);

	AttackHitBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AttackBoxOnBeginOverlap);
	
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::DetectSphereOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bSweepFrom, const FHitResult& SweepResults)
{
	if(!bIsAlive)
		return;

	AMainCharacter* MyChar = Cast<AMainCharacter>(OtherActor);

	if(MyChar)
	{
		TargetChar = MyChar;
		MoveToTarget();
	}
}

void AEnemy::DetectSphereOnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(!bIsAlive)
		return;

	AMainCharacter* MyChar = Cast<AMainCharacter>(OtherActor);

	if(MyChar)
	{
		TargetChar = nullptr;
		if(AI_Controller)
			AI_Controller->StopMovement();
	}
}

void AEnemy::CombatSphereOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bSweepFrom, const FHitResult& SweepResults)
{
	if(!bIsAlive)
		return;
	
	AMainCharacter* MyChar = Cast<AMainCharacter>(OtherActor);

	if(MyChar)
	{
		bTargetInAttackRange = true;
		float AttackDelay = FMath::RandRange(AttackDelayMin, AttackDelayMax);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay);
	}
}

void AEnemy::CombatSphereOnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(!bIsAlive)
		return;
	
	AMainCharacter* MyChar = Cast<AMainCharacter>(OtherActor);

	if(MyChar)
	{
		bTargetInAttackRange = false;
	}
}

void AEnemy::AttackBoxOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bSweepFrom, const FHitResult& SweepResults)
{
	if(CanDetectDamageCollision)
	{
		AMainCharacter* MyChar = Cast<AMainCharacter>(OtherActor);

		if(MyChar)
		{
			CanDetectDamageCollision = false;
			MyChar->ApplyDamage();
		}
	}
}

void AEnemy::MoveToTarget()
{
	if(bAttacking)
		return;

	if(AI_Controller)
	{
		FNavPathSharedPtr NavPath;
		FAIMoveRequest MoveRequest;

		MoveRequest.SetGoalActor(TargetChar);
		MoveRequest.SetAcceptanceRadius(MoveToTargetRadius);

		AI_Controller->MoveTo(MoveRequest, &NavPath);
	}
}

void AEnemy::Attack()
{
	if(!bIsAlive)
		return;

	if(bAttacking)
		return;

	bAttacking = true;
	CanDetectDamageCollision = true;

	if(AI_Controller)
		AI_Controller->StopMovement();

	auto AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.15f);
		AnimInstance->Montage_JumpToSection(TEXT("Attack"), CombatMontage);
	}
}

void AEnemy::AttackEnded()
{
	bAttacking = false;
	CanDetectDamageCollision = false;

	if(TargetChar)
	{
		if(bTargetInAttackRange)
		{
			float AttackDelay = FMath::RandRange(AttackDelayMin, AttackDelayMax);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay);
		}
		else
		{
			MoveToTarget();
		}
	}
	else
	{
		bTargetInAttackRange = false;
	}
}

void AEnemy::ApplyDamage()
{
	Health -= 20;
	if(Health <= 0)
	{
		if(AI_Controller)
			AI_Controller->StopMovement();

		bIsAlive = false;
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::DisposeEnemy, 2.f);
	}
}

void AEnemy::DisposeEnemy()
{
	Destroy();
}

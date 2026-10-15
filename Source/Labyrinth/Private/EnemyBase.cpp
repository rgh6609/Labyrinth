// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "EnemyDataAsset.h"
#include "MainCharacter.h"
#include "LabyrinthGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AEnemyBase::AEnemyBase()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	// AI 컨트롤러 없이 움직이기 (몬스터가 많아도 가볍게)
	AutoPossessAI = EAutoPossessAI::Disabled;                    // 컨트롤러 자동 생성 안 함
	GetCharacterMovement()->bRunPhysicsWithNoController = true; // 컨트롤러 없어도 이동 처리

	// 이동 방향으로 정면을 회전시키는 옵션
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f);
}



// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();


	TargetPlayer = UGameplayStatics::GetPlayerPawn(this, 0);

	bIsAttacking = false;


	// 💡 데이터 에셋이 존재한다면 몬스터 고유의 스펙을 내 몸통에 동적으로 주입
	if (EnemyDataAsset)
	{
		const FEnemySpec& Spec = EnemyDataAsset->EnemySpec;

		CurrentHP = Spec.MaxHP;
		FinalAttackRange = Spec.AttackRange; // 내 사거리 숫자를 저장 (AI 컨트롤러가 읽어감)
		AttackDamage = Spec.AttackDamage;
		FinalMiniAttackRange = Spec.MiniAttackRange;
		ContactDamageCooldown = Spec.AttackCooldown; // 접촉 데미지 간격

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = Spec.MoveSpeed;
		}
	}

	// 💡 [웨이브] 기본 능력치(데이터 에셋 또는 BP 기본값)에 웨이브 배율을 곱합니다.
	CurrentHP *= HealthMultiplier;
	AttackDamage *= DamageMultiplier;
	GetCharacterMovement()->MaxWalkSpeed *= SpeedMultiplier;

	// 1. [최적화] 매번 검색하지 않도록 0번 플레이어(타깃) 주소를 메모리에 미리 저장합니다.
	TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// 2. 처음 스폰되면 무조건 추적(Chase) 상태로 시작합니다.
	EnemyState = EEnemyState::Chase;

	// 3. 💡 0.1초마다 상황을 직접 판단하고 행동하는 경량 AI 타이머(뇌)를 작동시킵니다!
	GetWorldTimerManager().SetTimer(AIUpdateTimerHandle, this, &AEnemyBase::UpdateAIBehavior, 0.1f, true);

	// 4. 💡 [접촉 데미지] 캡슐이 무언가에 부딪히면 OnCapsuleHit이 호출되도록 연결
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AEnemyBase::OnCapsuleHit);
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState == EEnemyState::Dead) return;

	// 플레이어를 아직 못 찾았으면 다시 찾기 (몬스터가 플레이어보다 먼저 생성된 경우)
	if (!TargetPlayer)
	{
		TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (!TargetPlayer) return;
	}

	// 매 프레임 플레이어 쪽으로 밀어줌 → 도착 개념이 없어서 절대 멈추지 않고 계속 추적
	FVector MoveDirection = TargetPlayer->GetActorLocation() - GetActorLocation();
	MoveDirection.Z = 0.0f;
	AddMovementInput(MoveDirection.GetSafeNormal(), 1.0f);

	// 현재 상태를 추격(Chase) 상태로 명시해 줍니다. (애니메이션 연동용)
	EnemyState = EEnemyState::Chase;
}

void AEnemyBase::SetWaveMultipliers(float InHealthMultiplier, float InDamageMultiplier, float InSpeedMultiplier)
{
	// BeginPlay보다 먼저 호출되어야 체력 등이 배율대로 시작합니다. (GameMode의 SpawnActorDeferred 사용)
	HealthMultiplier = InHealthMultiplier;
	DamageMultiplier = InDamageMultiplier;
	SpeedMultiplier = InSpeedMultiplier;
}

void AEnemyBase::UpdateAIBehavior()
{

}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 이미 죽었거나 데미지가 0이면 무시 (투사체 여러 발이 동시에 맞아도 한 번만 죽도록)
	if (EnemyState == EEnemyState::Dead || ActualDamage <= 0.0f) return 0.0f;

	CurrentHP -= ActualDamage;
	if (CurrentHP <= 0.0f)
	{
		Die();
	}
	return ActualDamage;
}

void AEnemyBase::Die()
{
	// 먼저 Dead로 바꿔서 이동(Tick), 조준(스킬 컴포넌트), 피격(투사체)에서 모두 제외
	EnemyState = EEnemyState::Dead;

	// 1. 플레이어에게 경험치 지급 (데이터 에셋의 RewardExp, 데이터 에셋이 없으면 10)
	if (AMainCharacter* Player = Cast<AMainCharacter>(TargetPlayer))
	{
		Player->GainExp(EnemyDataAsset ? EnemyDataAsset->EnemySpec.RewardExp : 10.0f);
	}

	// 2. GameMode에 처치 수 알림 (결과 창의 킬 수)
	if (ALabyrinthGameModeBase* GM = GetWorld()->GetAuthGameMode<ALabyrinthGameModeBase>())
	{
		GM->OnEnemyKilled();
	}

	// 3. 필드에서 제거 → GameMode의 살아있는 몬스터 수도 자동으로 줄어듭니다 (스폰 시 OnDestroyed 연결)
	Destroy();
}

void AEnemyBase::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (EnemyState == EEnemyState::Dead) return;

	// 부딪힌 상대가 플레이어 캐릭터일 때만 (바닥, 벽, 다른 몬스터는 무시)
	AMainCharacter* Player = Cast<AMainCharacter>(OtherActor);
	if (!Player) return;

	// 쿨타임이 안 지났으면 무시 (몬스터가 계속 플레이어 쪽으로 밀고 있어서 붙어 있는 동안 매 프레임 Hit이 들어옴)
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastContactDamageTime < ContactDamageCooldown) return;
	LastContactDamageTime = Now;

	// 웨이브 배율이 적용된 공격력으로 데미지 (체력 0이 되면 MainCharacter::Die → 게임오버)
	Player->TakeSurvivalDamage(AttackDamage);
}
// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

/*oid AEnemyBase::StartAttack_Implementation()
{
	if (bIsAttacking) return;
	bIsAttacking = true;

	// 공격 모션을 취해야 하므로 이동을 즉시 정지합니다.
	GetCharacterMovement()->StopMovementImmediately();

	// 🛠️ 디버그 로그: 사거리에 정상 도달했는지 확인하기 위해 노란 글씨를 띄웁니다.
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("적: 사거리 도달! 공격 행동 개시!"));
	}
	// 💡 [선후딜 테스트 가드] 실제 애니메이션 몽타주가 꽂혀있다면 재생합니다.
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);

		// 실제 애니메이션 길이만큼 타이머를 재서, 애니메이션이 끝나면 EndAttack이 호출되게 만듭니다.
		float Duration = AttackMontage->GetPlayLength();
		GetWorldTimerManager().SetTimer(AttackDurationTimerHandle, this, &AEnemyBase::EndAttack, Duration, false);
	}
	else
	{
		// 💡 지금처럼 애니메이션이 없는 초기 단계라면, 데이터 에셋에 적힌 지속시간(혹은 기본 0.5초) 뒤에 
		// 자동으로 EndAttack이 실행되어 공격 잠금이 풀리도록 안전장치를 걸어둡니다.
		float Duration = (EnemyDataAsset) ? EnemyDataAsset->EnemySpec.AttackDuration : 0.5f;
		GetWorldTimerManager().SetTimer(AttackDurationTimerHandle, this, &AEnemyBase::EndAttack, Duration, false);
	}
}*/

// 💡 공격이 완전히 종료되었을 때 호출되어 상태 잠금을 풀어주는 함수
/*void AEnemyBase::EndAttack()
{
	bIsAttacking = false;

	if (EnemyState != EEnemyState::Dead)
	{
		EnemyState = EEnemyState::Idle; // 순간적으로 Idle로 돌려 다음 타이머 루프 때 재판단을 유도합니다.
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("적: 공격 선후딜 종료! 상태 리셋"));
	}
}*/

//예비 
void AEnemyBase::ApplyEffectDamage()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn || !EnemyDataAsset) return;

	// 플레이어가 아직 내 사거리 근처에 서 있는지 최종 확인
	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance <= FinalAttackRange + 50.0f) // 오차 범위를 살짝 감안
	{
		// 언리얼 표준 데미지 시스템 가동 (플레이어 피 깎기)
		UGameplayStatics::ApplyDamage(PlayerPawn, EnemyDataAsset->EnemySpec.AttackDamage, GetController(), this, UDamageType::StaticClass());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("적: 긁기 공격 적중! 데미지: %.1f"), EnemyDataAsset->EnemySpec.AttackDamage));
		}
	}

	// 공격 행동이 1회 완전히 끝났으므로 다시 움직일 수 있게 스위치를 풀어줍니다.
	bIsAttacking = false;
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "EnemyDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AEnemyBase::AEnemyBase()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 이동 방향으로 정면을 회전시키는 옵션
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}



// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = false;
	bIsAttacking = false;


	// 💡 데이터 에셋이 존재한다면 몬스터 고유의 스펙을 내 몸통에 동적으로 주입
	if (EnemyDataAsset)
	{
		const FEnemySpec& Spec = EnemyDataAsset->EnemySpec;

		CurrentHP = Spec.MaxHP;
		FinalAttackRange = Spec.AttackRange; // 내 사거리 숫자를 저장 (AI 컨트롤러가 읽어감)
		AttackDamage = Spec.AttackDamage;
		FinalMiniAttackRange = Spec.MiniAttackRange;

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = Spec.MoveSpeed;
		}
	}
	
	// 1. [최적화] 매번 검색하지 않도록 0번 플레이어(타깃) 주소를 메모리에 미리 저장합니다.
	TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// 2. 처음 스폰되면 무조건 추적(Chase) 상태로 시작합니다.
	EnemyState = EEnemyState::Chase;

	// 3. 💡 0.1초마다 상황을 직접 판단하고 행동하는 경량 AI 타이머(뇌)를 작동시킵니다!
	GetWorldTimerManager().SetTimer(AIUpdateTimerHandle, this, &AEnemyBase::UpdateAIBehavior, 0.1f, true);
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyBase::UpdateAIBehavior()
{
	// 🔴 중요: 죽었거나, 이미 공격 잠금 플래그(bIsAttacking)가 켜져 있다면 모든 이동과 거리 체크를 무시하고 대기합니다.
	// 이 잠금 덕분에 플레이어가 도망쳐도 몹이 미끄러지지 않고 하던 공격 모션(선후딜)을 끝까지 유지합니다.
	if (EnemyState == EEnemyState::Dead || EnemyState == EEnemyState::Attack || bIsAttacking)
	{
		return;
	}
	
	if (!TargetPlayer)
	{
		TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (!TargetPlayer) return;
	}

	float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

	if (Distance < FinalMiniAttackRange)
	{
		EnemyState = EEnemyState::Run;

		FVector EscapeDirection = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal2D() * -1.0f;
		AddMovementInput(EscapeDirection, 1.0f);

		FVector LookDirection = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		FRotator TargetRotation = LookDirection.Rotation();
		SetActorRotation(FRotator(0.f, TargetRotation.Yaw, 0.f));
	}
	else if (Distance <= FinalAttackRange)
	{
		EnemyState = EEnemyState::Attack;
		StartAttack();
	}
	// 🔴 [판단 1] 정확히 사거리 안으로 진입했다면? -> 멈추고 공격 돌입
	if (Distance <= FinalAttackRange)
	{
		EnemyState = EEnemyState::Attack;
		StartAttack(); // 공격 개시!
	}
	// 🔴 [판단 2] 사거리 밖에 있다면? -> 플레이어를 향해 단순 직선 전진 (재추적 자동 해결)
	else
	{
		EnemyState = EEnemyState::Chase;

		// 길찾기(NavMesh) 연산을 거치지 않는 초경량 직선 이동 입력 (렉 유발 주범 제거)
		FVector Direction = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		AddMovementInput(Direction, 1.0f);

		// CharacterMovement의 bOrientRotationToMovement 옵션 덕분에 회전은 엔진이 알아서 부드럽게 처리해 줍니다.
	}
}
// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyBase::StartAttack_Implementation()
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
}

// 💡 공격이 완전히 종료되었을 때 호출되어 상태 잠금을 풀어주는 함수
void AEnemyBase::EndAttack()
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
}

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


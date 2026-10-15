// Fill out your copyright notice in the Description page of Project Settings.

#include "MSSkillComponent.h" // 👈 필수 헤더 포함
#include "MSProjectile.h"
#include "EnemyBase.h"
#include "MainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

UMSSkillComponent::UMSSkillComponent()
{
	// 타겟팅과 발사는 타이머(Timer)로 정밀하게 제어하므로, 매 프레임 도는 Tick은 꺼서 최적화합니다.
	PrimaryComponentTick.bCanEverTick = false;
}

void UMSSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// 게임이 시작되면(또는 스킬을 획득해서 컴포넌트가 붙으면) 지정된 주기(AttackInterval)마다 FireProjectile 함수를 무한 반복 실행합니다.
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &UMSSkillComponent::FireProjectile, FMath::Max(0.1f, AttackInterval), true);
}

void UMSSkillComponent::InitSkill(const FMSUpgradeData& Data)
{
	// 데이터 테이블에 적힌 이 스킬의 기본값으로 세팅
	ProjectileClass = Data.ProjectileClass;
	SkillDamage = Data.BaseDamage;
	AttackInterval = Data.AttackInterval;

	SkillLevel = 1;
	ProjectileCount = 1;
}

void UMSSkillComponent::LevelUpSkill()
{
	SkillLevel++;
	SkillDamage *= 1.03f; // 데미지 +3%
	ProjectileCount++;    // 투사체 +1
}

TArray<AActor*> UMSSkillComponent::FindClosestEnemies(int32 Count)
{
	TArray<AActor*> Result;

	AActor* Owner = GetOwner();
	if (!Owner) return Result;

	const FVector OwnerLocation = Owner->GetActorLocation();

	// 월드에 있는 몬스터(EnemyBase)만 가져옵니다. 플레이어나 다른 Pawn은 조준하지 않습니다.
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);

	for (AActor* Enemy : FoundEnemies)
	{
		// 이미 죽은 몬스터(사망 처리 중)는 조준하지 않습니다.
		if (Cast<AEnemyBase>(Enemy)->EnemyState == EEnemyState::Dead) continue;

		// 탐색 반경 밖의 몬스터는 제외
		if (FVector::Dist(OwnerLocation, Enemy->GetActorLocation()) > ScanRadius) continue;

		Result.Add(Enemy);
	}

	// 가까운 순으로 정렬한 뒤 앞에서 Count마리만 남깁니다.
	Result.Sort([&OwnerLocation](const AActor& A, const AActor& B)
	{
		return FVector::DistSquared(OwnerLocation, A.GetActorLocation()) < FVector::DistSquared(OwnerLocation, B.GetActorLocation());
	});
	if (Result.Num() > Count)
	{
		Result.SetNum(Count);
	}

	return Result;
}

void UMSSkillComponent::FireProjectile()
{
	AActor* Owner = GetOwner();
	if (!Owner || !ProjectileClass) return;

	// 가까운 적부터 투사체 개수만큼 탐색합니다. (매직서바이벌처럼 여러 적을 한 발씩 조준)
	const TArray<AActor*> Targets = FindClosestEnemies(ProjectileCount);
	if (Targets.Num() == 0) return; // 주변에 적이 한 마리도 없다면 무기를 쏘지 않고 패스합니다.

	// 캐릭터의 데미지 배율 (능력치 카드 Stat_Damage로 증가, 모든 스킬에 공통 적용)
	float DamageMultiplier = 1.0f;
	if (const AMainCharacter* Character = Cast<AMainCharacter>(Owner))
	{
		DamageMultiplier = Character->DamageMultiplier;
	}

	const FVector SpawnLocation = Owner->GetActorLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Cast<APawn>(Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < ProjectileCount; ++i)
	{
		// 투사체가 적보다 많으면 남는 발은 가장 가까운 적부터 다시 조준합니다.
		AActor* TargetEnemy = Targets[i % Targets.Num()];

		// 적이 있는 방향을 바라보도록 회전값(각도)을 계산합니다.
		FRotator SpawnRotation = (TargetEnemy->GetActorLocation() - SpawnLocation).Rotation();

		// 투사체가 땅속으로 처박히거나 하늘로 솟지 않도록 2D 평면 회전(Yaw)만 남기고 고정합니다.
		SpawnRotation.Pitch = 0.0f;
		SpawnRotation.Roll = 0.0f;

		// 계산된 각도로 월드에 투사체를 생성(스폰)합니다.
		AMSProjectile* SpawnedProj = GetWorld()->SpawnActor<AMSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (SpawnedProj)
		{
			// 스킬 데미지 × 캐릭터 데미지 배율을 투사체에 주입합니다.
			SpawnedProj->Damage = SkillDamage * DamageMultiplier;
		}
	}
}

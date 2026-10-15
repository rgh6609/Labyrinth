// Fill out your copyright notice in the Description page of Project Settings.

#include "LabyrinthGameModeBase.h"
#include "EnemyBase.h"
#include "EnemyDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"

ALabyrinthGameModeBase::ALabyrinthGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentState = ESurvivalGameState::Ready;
}

void ALabyrinthGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	StartGame();
}

void ALabyrinthGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 오직 게임이 플레이 중일 때만 타이머를 작동시킵니다.
	if (CurrentState == ESurvivalGameState::Playing)
	{
		ElapsedTime += DeltaSeconds;

		// 목표 생존 시간을 버텼다면 승리 처리
		if (ElapsedTime >= StageDuration)
		{
			EndGame(ESurvivalGameState::GameClear);
		}
	}
}

void ALabyrinthGameModeBase::StartGame()
{
	CurrentState = ESurvivalGameState::Playing;
	ElapsedTime = 0.f;
	KillCount = 0;
	AliveEnemyCount = 0;
	CurrentWaveIndex = INDEX_NONE;
	SpawnTickCount = 0;

	// SpawnInterval초마다 SpawnEnemies를 무한 반복 (첫 스폰은 시작 1초 뒤, 플레이어가 생성될 시간을 줌)
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ALabyrinthGameModeBase::SpawnEnemies, SpawnInterval, true, 1.0f);
}

void ALabyrinthGameModeBase::SpawnEnemies()
{
	// 플레이 중이 아니면 스폰하지 않음
	if (CurrentState != ESurvivalGameState::Playing) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return;

	// --- 1. 웨이브 단계 계산: 스폰 SpawnsPerWave번마다 1단계씩 오름 ---
	const int32 NewWaveIndex = SpawnTickCount / SpawnsPerWave;
	SpawnTickCount++;

	if (NewWaveIndex != CurrentWaveIndex)
	{
		CurrentWaveIndex = NewWaveIndex;

		// 블루프린트에 웨이브가 바뀌었음을 알림 (UI 표시용)
		OnWaveChangedBlueprintEvent(CurrentWaveIndex);
	}

	// --- 2. 이번 단계의 마리 수와 능력치 배율 ---
	const int32 SpawnCount = BaseSpawnCount + CurrentWaveIndex * SpawnCountIncreasePerWave;
	const float HealthMultiplier = 1.0f + CurrentWaveIndex * HealthIncreasePerWave;
	const float DamageMultiplier = 1.0f + CurrentWaveIndex * DamageIncreasePerWave;
	const float SpeedMultiplier = FMath::Min(1.0f + CurrentWaveIndex * SpeedIncreasePerWave, MaxSpeedMultiplier);

	// --- 3. SpawnCount마리 소환 ---
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		// 몬스터 상한선에 도달했다면 그만 소환 (렉 방지)
		if (AliveEnemyCount >= MaxAliveEnemies) break;

		// 데이터 에셋의 출현 확률(SpawnWeight)과 등장 단계(MinSpawnWave)에 따라 종류 선택
		const TSubclassOf<AEnemyBase> SelectedEnemyClass = PickRandomEnemyClass();
		if (!SelectedEnemyClass) return; // 지금 단계에서 나올 수 있는 몬스터가 하나도 없음

		const FTransform SpawnTransform(FRotator::ZeroRotator, GetRandomSpawnLocation(PlayerLocation));

		// 💡 Deferred 스폰: 액터를 만들되 BeginPlay는 아직 실행하지 않은 상태로 받아옵니다.
		//    이 사이에 웨이브 배율을 넣어야, BeginPlay에서 데이터 에셋 값을 읽은 뒤 배율이 곱해집니다.
		AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActorDeferred<AEnemyBase>(SelectedEnemyClass, SpawnTransform,
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (!SpawnedEnemy) continue;

		SpawnedEnemy->SetWaveMultipliers(HealthMultiplier, DamageMultiplier, SpeedMultiplier);
		SpawnedEnemy->FinishSpawning(SpawnTransform); // 여기서 BeginPlay 실행

		AliveEnemyCount++;
		// 몬스터가 죽었을 때(파괴될 때) 감지하기 위해 델리게이트 바인딩
		SpawnedEnemy->OnDestroyed.AddDynamic(this, &ALabyrinthGameModeBase::HandleEnemyDestroyed);
	}
}

TSubclassOf<AEnemyBase> ALabyrinthGameModeBase::PickRandomEnemyClass() const
{
	// 1. 후보 고르기: 현재 웨이브 단계에서 나올 수 있고, 가중치가 0보다 큰 몬스터만
	TArray<TSubclassOf<AEnemyBase>> Candidates;
	TArray<float> Weights;
	float TotalWeight = 0.f;

	for (const TSubclassOf<AEnemyBase>& EnemyClass : EnemyClasses)
	{
		if (!EnemyClass) continue;

		// BP의 기본값(CDO)에 꽂혀 있는 데이터 에셋을 읽어옵니다.
		// 데이터 에셋이 없으면 가중치 10, 처음부터 등장으로 취급
		const UEnemyDataAsset* Data = EnemyClass->GetDefaultObject<AEnemyBase>()->EnemyDataAsset;
		const float Weight = Data ? Data->SpawnWeight : 10.f;
		const int32 MinSpawnWave = Data ? Data->MinSpawnWave : 0;

		if (Weight <= 0.f || CurrentWaveIndex < MinSpawnWave) continue;

		Candidates.Add(EnemyClass);
		Weights.Add(Weight);
		TotalWeight += Weight;
	}

	if (Candidates.Num() == 0) return nullptr;

	// 2. 가중치 랜덤: 0 ~ 총합 사이 숫자를 뽑고, 그 숫자가 어느 몬스터 구간에 떨어지는지 찾기
	//    예) 가중치 [50, 30, 20], 뽑은 숫자 65 → 50을 넘고 80(50+30) 안이므로 두 번째 몬스터
	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		if (Roll < Weights[i]) return Candidates[i];
		Roll -= Weights[i];
	}
	return Candidates.Last(); // 부동소수점 오차 대비
}

FVector ALabyrinthGameModeBase::GetRandomSpawnLocation(const FVector& Center) const
{
	// 스폰 범위
	float RandomAngle = FMath::RandRange(0.f, 360.f);
	float Rad = FMath::DegreesToRadians(RandomAngle);

	FVector Offset(FMath::Cos(Rad) * SpawnRadius, FMath::Sin(Rad) * SpawnRadius, 0.f);
	FVector SpawnPos = Center + Offset;

	// 바닥 높이 보정
	SpawnPos.Z = Center.Z;
	return SpawnPos;
}

void ALabyrinthGameModeBase::HandleEnemyDestroyed(AActor* DestroyedActor)
{
	// 살아있는 적 숫자 감소
	AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);
}

void ALabyrinthGameModeBase::OnEnemyKilled()
{
	if (CurrentState == ESurvivalGameState::Playing)
	{
		KillCount++;
	}
}

void ALabyrinthGameModeBase::OnPlayerDied()
{
	EndGame(ESurvivalGameState::GameOver);
}

void ALabyrinthGameModeBase::EndGame(ESurvivalGameState NewState)
{
	if (CurrentState != ESurvivalGameState::Playing) return;

	CurrentState = NewState;

	// 스폰 타이머 중지
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	// 게임 내 모든 요소 일시정지 
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// 상태에 따라 일치하는 블루프린트 이벤트
	if (CurrentState == ESurvivalGameState::GameOver)
	{
		OnGameOverBlueprintEvent();
	}
	else if (CurrentState == ESurvivalGameState::GameClear)
	{
		OnGameClearBlueprintEvent();
	}

	// 기존 코드가 가지고 있던 C++ 직접 UI 생성 방식 (서브 기능용 백업)
	if (ResultWidgetClass)
	{
		UUserWidget* ResultWidget = CreateWidget<UUserWidget>(GetWorld(), ResultWidgetClass);
		if (ResultWidget)
		{
			ResultWidget->AddToViewport();
			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (PC)
			{
				PC->SetShowMouseCursor(true);
			}
		}
	}
}
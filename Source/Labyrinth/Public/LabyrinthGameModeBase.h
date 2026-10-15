// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h"
#include "LabyrinthGameModeBase.generated.h"

class UUserWidget;
class AEnemyBase;
class AMSProjectile;

// 게임의 진행 상태를 나타내는 열거형
UENUM(BlueprintType)
enum class ESurvivalGameState : uint8
{
	Ready,
	Playing,
	GameOver,
	GameClear
};

// 강화 카드 종류
UENUM(BlueprintType)
enum class EMSUpgradeType : uint8
{
	Stat         UMETA(DisplayName = "능력치"),   // UpgradeID로 구분: Stat_Damage / Stat_MaxHP / Stat_MoveSpeed
	AttackSkill  UMETA(DisplayName = "공격 스킬")  // 없으면 획득, 있으면 레벨업 (데미지 +3%, 투사체 +1)
};

// ⭐ [매직 서바이벌 스킬 데이터 구조체 추가] (SkillDataTable의 한 행 = 강화 카드 한 장)
USTRUCT(BlueprintType)
struct FMSUpgradeData : public FTableRowBase
{
	GENERATED_BODY()

	// 스킬 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FText UpgradeName;

	// 스킬 설명 (예: "공격력이 10% 증가합니다.")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FText Description;

	// UI에 표시될 이쁜 스킬 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	UTexture2D* IconTexture = nullptr;

	// 내부 코드 구분을 위한 스킬 태그 ID (예: "Stat_Damage", "Skill_Fireball")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FName UpgradeID;

	// 카드 종류: 능력치 / 공격 스킬
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	EMSUpgradeType UpgradeType = EMSUpgradeType::Stat;

	// --- [능력치 카드용] ---
	// 증가량 (Stat_Damage: 0.2 = 데미지 +20% / Stat_MoveSpeed: 0.1 = 이동속도 +10% / Stat_MaxHP: 20 = 최대 체력 +20)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade|Stat")
	float Value = 0.2f;

	// --- [공격 스킬 카드용] ---
	// 발사할 투사체 BP (BP_MagicBolt, BP_Fireball ...)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade|Skill")
	TSubclassOf<AMSProjectile> ProjectileClass;

	// 획득 시 기본 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade|Skill")
	float BaseDamage = 25.0f;

	// 발사 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade|Skill", meta = (ClampMin = "0.1"))
	float AttackInterval = 1.5f;

	// 최대 레벨 (도달하면 더 이상 카드로 안 나옴)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade|Skill", meta = (ClampMin = "1"))
	int32 MaxLevel = 10;
};

UCLASS()
class LABYRINTH_API ALabyrinthGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALabyrinthGameModeBase();

	// 플레이어 캐릭터가 체력 0이 되면 호출
	UFUNCTION(BlueprintCallable, Category = "Survival|Gameplay")
	void OnPlayerDied();

	// 적이 플레이어에게 죽었을 때 적 쪽에서 호출
	UFUNCTION(BlueprintCallable, Category = "Survival|Gameplay")
	void OnEnemyKilled();

	// --- [ 블루프린트용 Pure Get 함수들 ] ---
	UFUNCTION(BlueprintPure, Category = "Survival|Stats")
	float GetElapsedTime() const { return ElapsedTime; }

	UFUNCTION(BlueprintPure, Category = "Survival|Stats")
	float GetRemainingTime() const { return FMath::Max(0.f, StageDuration - ElapsedTime); }

	// 현재 웨이브 단계 (0부터 시작, SpawnsPerWave번 스폰할 때마다 1씩 오름)
	UFUNCTION(BlueprintPure, Category = "Survival|Stats")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	UFUNCTION(BlueprintPure, Category = "Survival|Stats")
	int32 GetKillCount() const { return KillCount; }

	UFUNCTION(BlueprintPure, Category = "Survival|Stats")
	ESurvivalGameState GetCurrentState() const { return CurrentState; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// --- [ 에디터 세팅 변수 ] ---
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Stage")
	float StageDuration = 900.f;

	// --- [ 스폰 설정 ] ---

	// 스폰할 몬스터 BP 목록
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Spawn")
	TArray<TSubclassOf<AEnemyBase>> EnemyClasses;

	// 몇 초마다 스폰할지
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Spawn", meta = (ClampMin = "0.1"))
	float SpawnInterval = 10.f;

	// 웨이브 0단계에서 한 번에 스폰할 마리 수
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Spawn", meta = (ClampMin = "1"))
	int32 BaseSpawnCount = 4;

	// 플레이어로부터 이 거리(화면 밖)의 원 위에서 스폰
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Spawn")
	float SpawnRadius = 1500.f;

	// 동시에 살아있을 수 있는 최대 몬스터 수 (렉 방지 상한선)
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Spawn")
	int32 MaxAliveEnemies = 300;

	// --- [ 웨이브(난이도) 설정 ] ---
	

	//웨이브 단계
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Wave", meta = (ClampMin = "1"))
	int32 SpawnsPerWave = 3;

	// 단계마다 한 번에 나오는 마리 수 증가량
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Wave", meta = (ClampMin = "0"))
	int32 SpawnCountIncreasePerWave = 2;

	// 단계마다 체력 증가 비율
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Wave", meta = (ClampMin = "0"))
	float HealthIncreasePerWave = 0.15f;

	// 단계마다 공격력 증가 비율 
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Wave", meta = (ClampMin = "0"))
	float DamageIncreasePerWave = 0.10f;

	// 단계마다 이동속도 증가 비율
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Wave", meta = (ClampMin = "0"))
	float SpeedIncreasePerWave = 0.03f;

	// 이동속도 배율 상한 
	UPROPERTY(EditDefaultsOnly, Category = "Survival|Wave", meta = (ClampMin = "1"))
	float MaxSpeedMultiplier = 1.5f;

	// C++ UI 직접 생성용 (보험용으로 남겨둠)
	UPROPERTY(EditDefaultsOnly, Category = "Survival|UI")
	TSubclassOf<UUserWidget> ResultWidgetClass;

	// --- [ 업그레이드: 블루프린트 UI 연동용 이벤트 ] ---
	// C++에서 이 이벤트들을 호출하면 블루프린트 이벤트를 통해 화려한 UMG UI를 쉽게 띄울 수 있습니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Survival|UI")
	void OnGameOverBlueprintEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Survival|UI")
	void OnGameClearBlueprintEvent();

	// 웨이브 단계가 오를 때 호출 (UI에 "WAVE 3" 같은 표시용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Gameplay")
	void OnWaveChangedBlueprintEvent(int32 NewWaveIndex);

private:
	void StartGame();
	void EndGame(ESurvivalGameState NewState);
	void SpawnEnemies();

	// 현재 웨이브 단계에서 나올 수 있는 몬스터 중 가중치 랜덤으로 하나 뽑기 (없으면 nullptr)
	TSubclassOf<AEnemyBase> PickRandomEnemyClass() const;
	FVector GetRandomSpawnLocation(const FVector& Center) const;

	UFUNCTION()
	void HandleEnemyDestroyed(AActor* DestroyedActor);

	FTimerHandle SpawnTimerHandle;
	float ElapsedTime = 0.f;
	int32 CurrentWaveIndex = INDEX_NONE;

	// 지금까지 스폰한 횟수 (웨이브 단계 계산용)
	int32 SpawnTickCount = 0;
	int32 AliveEnemyCount = 0;
	int32 KillCount = 0;

	// bool 변수들을 직관적인 열거형 하나로 대체
	ESurvivalGameState CurrentState = ESurvivalGameState::Ready;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MSSkillComponent.generated.h"

class AMSProjectile;
struct FMSUpgradeData;

// 💡 공격 스킬 1개 = 이 컴포넌트 1개
//    스킬 카드를 처음 고르면 MainCharacter가 이 컴포넌트를 새로 붙이고(InitSkill),
//    같은 카드를 또 고르면 LevelUpSkill()로 강해집니다.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LABYRINTH_API UMSSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMSSkillComponent();

	// 💡 [스킬 획득] 데이터 테이블 행의 값으로 이 스킬을 세팅합니다. (RegisterComponent 전에 호출)
	void InitSkill(const FMSUpgradeData& Data);

	// 💡 [스킬 레벨업] 데미지 +3%, 투사체 +1
	void LevelUpSkill();

	UFUNCTION(BlueprintPure, Category = "Skill")
	int32 GetSkillLevel() const { return SkillLevel; }

protected:
	virtual void BeginPlay() override;

	// 반복 발사를 통제할 타이머 핸들
	FTimerHandle AttackTimerHandle;

	// 발사할 투사체 블루프린트 클래스 지정을 위한 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<AMSProjectile> ProjectileClass;

	// 스킬 능력치 (기본 세팅 값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Stats")
	float AttackInterval = 1.5f; // 발사 쿨타임 (초)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Stats")
	float SkillDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Stats")
	float ScanRadius = 1500.0f; // 적을 탐색할 최대 반경

	// 한 번에 발사하는 투사체 수 (레벨업마다 +1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Stats")
	int32 ProjectileCount = 1;

	// 현재 스킬 레벨 (획득 시 1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Stats")
	int32 SkillLevel = 1;

	// 탐색 반경 안의 살아있는 적을 가까운 순으로 최대 Count마리 찾기
	TArray<AActor*> FindClosestEnemies(int32 Count);

	// 실제로 투사체를 소환하여 발사하는 함수
	void FireProjectile();
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyDataAsset.generated.h"


//몬스터 타입
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    Regular     UMETA(DisplayName = "일반형"),
    Ranged      UMETA(DisplayName = "원거리형"),
 // Rush        UMETA(DisplayName = "돌진형"),
//  Special     UMETA(DisplayName = "특수형")
}; 
//몬스터 능력치
USTRUCT(BlueprintType)
struct FEnemySpec
{
    GENERATED_BODY()


    // --- 1. 전투 및 이동 스탯 (Combat & Movement) ---
    //체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
    float MaxHP = 10.0f;

    //이동 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
    float MoveSpeed = 300.0f;

    //사거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
    float AttackRange = 50.0f;

    //데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
    float AttackDamage = 1.0f;
    
    // 공격 쿨타임 (초)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemyStats")
    float AttackCooldown = 2.0f; 

    // 임시 딜레이 데이터 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterStats")
    float AttackDuration = 0.5f;
    
    //최소 사거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterStats")
    float MiniAttackRange = 0.5f;
    

    // --- 3. 보상 시스템 데이터 (Rewards) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
    int32 RewardExp = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
    int32 MinRewardGold = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
    int32 MaxRewardGold = 30;


    // --- 4. 메타데이터 (UI 등) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile")
    FText EnemyName;

};

// 💡 에디터에서 마우스 우클릭 -> 기타 -> 데이터 에셋으로 뽑아낼 진짜 파일 규격입니다.
UCLASS(BlueprintType)
class LABYRINTH_API UEnemyDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemyData")
    FEnemySpec EnemySpec;

    // 💡 [추가] 이 몬스터 전용 공격/사망 애니메이션 몽타주 등록 슬롯
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "animation")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "animation")
    class UAnimMontage* DeathMontage;
};

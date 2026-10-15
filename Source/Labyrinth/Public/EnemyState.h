// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EnemyState.generated.h"

class AEnemyAIController;
class AEnemyBase;


UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Idle,           // 대기
    Patrol,         // 순찰
    Chase,          // 추격
    Run,            // 도망( 사거리 유지 )
    Attack,         // 일반 공격 (평타)
    Dead            // 사망
};

UCLASS()
class LABYRINTH_API UEnemyState : public UObject
{
    GENERATED_BODY()
};


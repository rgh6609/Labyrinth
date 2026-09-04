// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyState.h"
#include "EnemyBase.generated.h"



UCLASS()
class LABYRINTH_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

protected:
	
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

    //공격 개시 스위치
    UFUNCTION(BlueprintNativeEvent, Category = "Enemy|Combat")
    void StartAttack();
    virtual void StartAttack_Implementation();

    // 💡 [추가] 공격 선후딜이 끝났을 때(가상 타이머나 애님 노티파이 종료 시) 호출하여 잠금을 풀 함수
    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void EndAttack();

    // 💡 애니메이션에서 실제 타격 판정이 일어나는 순간 호출할 함수 (기존 구조 유지)
    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void ApplyEffectDamage();

    // 💡 에디터에서 몬스터 종류별 카드를 꽂아줄 슬롯 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStats")
    class UEnemyDataAsset* EnemyDataAsset;
    
    // 능력치 저장 변수
    
    //체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStats")
    float CurrentHP;
    
    //사거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStats")
    float FinalAttackRange;

    //데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
    float AttackDamage;

    // 💡 [추가] 이 거리보다 플레이어가 가까워지면 뒤로 물러납니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
    float FinalMiniAttackRange = 80.0f; // 기본값 80 (데이터 에셋에서 받아와도 됩니다)
     
    // --- 애니메이션 관련 ---
    UPROPERTY(EditAnywhere, Category = "Effects")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, Category = "Effects")
    UAnimMontage* DeathMontage;

    // 💡 [수정] 외부에서 들고 오는 EEnemyState 열거형 변수로 연동
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EEnemyState EnemyState = EEnemyState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsAttacking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float SphereRadius = 60.f;


private:
    // 💡 [최적화] 매 프레임 찾으면 렉 걸리는 플레이어 주소를 미리 저장해둘 변수
    UPROPERTY()
    APawn* TargetPlayer;

    // 💡 0.1초마다 대가리를 굴리며 거리를 체크할 경량 AI 타이머 핸들
    FTimerHandle AIUpdateTimerHandle;

    // 💡 애니메이션이 없을 때 가상으로 공격 지속 시간(선후딜)을 재어줄 타이머 핸들
    FTimerHandle AttackDurationTimerHandle;

    // 💡 0.1초마다 무한 반복 실행되며 상태와 거리를 판단할 핵심 두뇌 함수
    UFUNCTION()
    void UpdateAIBehavior();
};

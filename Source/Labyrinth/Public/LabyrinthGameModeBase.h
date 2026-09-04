// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LabyrinthGameModeBase.generated.h"

/**
 * 
 */

//class UEnemySpawnComponent;

UCLASS()
class LABYRINTH_API ALabyrinthGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	

public:
	ALabyrinthGameModeBase();

  // 게임 시스템 관련

	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void StartGame(FName LevelName);

	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void GameOver();

	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void VictoryClear();

	// 재화관련 
	//UFUNCTION(BlueprintCallable, Category = "GameSystem")
	//void GameClear(int32 inCurrentStage);

	//점수
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	//int32 CurrentScore = 0;

	//UFUNCTION(BlueprintCallable, Category = "GameSystem")
	//void AddScore(int32 Amount);

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	//class UEnemySpawnComponent* EnemySpawnComp;

	UPROPERTY()
	TArray<AActor*> SpawnPoints;

protected:

	virtual void BeginPlay() override;

	
	//난이도 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	UDataTable* EnemyWaveTable;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//void Gamelevel(int32 CurrentStage);

public:

	//시간관련	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSystem")
	FTimerHandle MainTimerHandle;

	//UFUNCTION(BlueprintCallable, Category = "GameSystem")
	//void UpdateGameTime();

	// 현재 경과 시간 (초 단위)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSystem")
	int32 ElapsedSeconds = 0;

	// 현재 진행 중인 웨이브 단계
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameSystem")
	int32 CurrentWaveIndex = 0;

	// 데이터 테이블에서 가져온 전체 웨이브 스케줄 정보 배열
	TArray<struct FWaveData*> WaveSchedule;

	// 새로운 웨이브가 시작될 때 스폰러에 신호를 주는 함수
	//UFUNCTION(BlueprintCallable, Category = "GameSystem")
	//void CheckAndTriggerNextWave();
};

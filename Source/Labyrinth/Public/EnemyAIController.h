// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnemyBase.h"
#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyAIController.generated.h"

UCLASS()
class LABYRINTH_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

protected:
	// 💡 몹이 월드에 스폰될 때 딱 한 번 영혼을 넣어줄 빙의 함수
	virtual void OnPossess(APawn* InPawn) override;

	// 💡 헤더 선언은 그대로 유지하되, 내부 로직은 전부 몬스터 몸통(EnemyBase)으로 이사시켰으므로 .cpp에서 비워둘 것입니다.
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	// 💡 더 이상 컨트롤러 자체 타이머는 쓰지 않으므로 이 변수는 보관만 해둡니다.
	FTimerHandle ChaseTimerHandle;
};

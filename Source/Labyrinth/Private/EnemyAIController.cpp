// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "EnemyBase.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 💡 이제 컨트롤러는 내부에서 직접 MoveTo(길찾기) 명령을 내리지 않습니다.
	// 몬스터 몸통(EnemyBase)의 BeginPlay에서 구동되는 0.1초 타이머가 
	// 거리를 직접 계산하고 AddMovementInput으로 부드럽게 밀어주기 때문입니다.
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("AIController_Log: 경량 AI 컨트롤러 빙의 완료!"));
	}
}

// 💡 관성 오차와 헛손질 버그를 유발하던 엔진 내장 이동 완료 이벤트는 기능을 비워둡니다.
void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	// 부모 클래스의 기본 연산만 실행해주고, 내부에서 StartAttack()을 강제로 때려 꼬이게 만들던 부분은 삭제합니다.
	Super::OnMoveCompleted(RequestID, Result);
}
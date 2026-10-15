// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class LABYRINTH_API AMSProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMSProjectile();

protected:
	virtual void BeginPlay() override;

	// 충돌을 감지할 구체 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComp;

	// 투사체를 날려줄 무브먼트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	// 몬스터와 부딪혔을 때 실행될 함수
	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	// 스킬 데미지 (스킬 시스템에서 투사체를 생성할 때 값을 채워줄 예정)
	UPROPERTY(BlueprintReadWrite, Category = "Projectile|Stats")
	float Damage = 20.0f;

private:
	// 이미 한 마리를 맞혔는지 (한 프레임에 여러 몬스터와 동시에 겹쳐도 한 마리만 맞도록)
	bool bHasHit = false;
};
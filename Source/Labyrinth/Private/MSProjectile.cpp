// Fill out your copyright notice in the Description page of Project Settings.


#include "MSProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyBase.h"

AMSProjectile::AMSProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. 충돌체 설정
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Trigger")); // 겹침 이벤트 발생 프로필
	RootComponent = CollisionComp;

	// 2. 투사체 무브먼트 
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->bRotationFollowsVelocity = true; // 날아가는 방향을 바라보도록 설정
	ProjectileMovement->ProjectileGravityScale = 0.0f;   // 
}

void AMSProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 충돌(Overlap) 이벤트 연결
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AMSProjectile::OnProjectileOverlap);

	// 3초 뒤에 적을 못 맞춰도 메모리 확보를 위해 자동 파괴 (사거리 제한 역할)
	SetLifeSpan(3.0f);
}

void AMSProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 살아있는 몬스터에게만 반응합니다. (플레이어, 다른 투사체, 벽 등은 그냥 통과)
	AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
	if (bHasHit || !Enemy || Enemy->EnemyState == EEnemyState::Dead) return;

	// 한 번에 여러 마리와 겹쳐도 한 마리만 맞도록 잠금
	bHasHit = true;

	// 언리얼 표준 데미지 함수 → 몬스터의 TakeDamage로 전달됩니다.
	// (몬스터 쪽 TakeDamage는 D단계에서 구현 예정. 그 전까지는 맞아도 체력이 줄지 않음)
	UGameplayStatics::ApplyDamage(Enemy, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

	// 적과 부딪혔으므로 투사체 소멸
	Destroy();
}



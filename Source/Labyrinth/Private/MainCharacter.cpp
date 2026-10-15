// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "MSSkillComponent.h"
#include "LabyrinthPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 컨트롤러 회전이 캐릭터/카메라에 영향 주지 않도록
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 이동 방향으로 캐릭터 몸통이 돌아가도록
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

	// 매직서바이벌처럼 즉각 반응하는 이동감
	MoveComp->MaxWalkSpeed = 500.0f;
	MoveComp->MaxAcceleration = 10000.0f;           // 바로 최고속도
	MoveComp->BrakingDecelerationWalking = 10000.0f; // 키 떼면 바로 멈춤
	MoveComp->GroundFriction = 8.0f;

	// ===== 고정 탑다운 카메라 =====
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);   // 캐릭터가 돌아도 카메라는 안 돎
	CameraBoom->SetRelativeRotation(FRotator(CameraPitch, 0.0f, 0.0f));
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bUsePawnControlRotation = false;  // 마우스/컨트롤러로 안 돎
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bDoCollisionTest = false;         // 벽 때문에 카메라가 당겨지지 않게

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 에디터에서 바꾼 카메라 값 반영
	CameraBoom->SetWorldRotation(FRotator(CameraPitch, 0.0f, 0.0f));
	CameraBoom->TargetArmLength = CameraDistance;

	CurrentHP = MaxHP;

	// 💡 게임 시작하자마자 시작 공격 스킬 선택 (공격 스킬 카드만 나옴)
	//    다음 프레임에 여는 이유: 컨트롤러 빙의와 GameMode 시작 처리가 끝난 뒤에 일시정지하기 위해
	bStartingChoice = true;
	GetWorldTimerManager().SetTimerForNextTick(this, &AMainCharacter::OpenUpgradeMenu);
}

void AMainCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// 입력 매핑 컨텍스트 등록 (이게 없으면 WASD가 안 들어옴)
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacter::Move);
		}
	}
}


void AMainCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// 카메라가 바라보는 수평 방향(Yaw)만 사용
	const FRotator YawRotation(0.0f, FollowCamera->GetComponentRotation().Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y); // W, S
	AddMovementInput(RightDirection, MovementVector.X);   // A, D
}
void AMainCharacter::TakeSurvivalDamage(float DamageAmount)
{
	// 🔄 ALabyrinthGameModeBase로 변경하여 현재 Playing 상태일 때만 데미지를 계산합니다.
	ALabyrinthGameModeBase* GM = Cast<ALabyrinthGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM && GM->GetCurrentState() != ESurvivalGameState::Playing) return;

	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

	// 체력이 0이 되면 사망
	if (CurrentHP <= 0.0f)
	{
		Die();
	}
}

void AMainCharacter::GainExp(float Amount)
{
	CurrentEXP += Amount;

	// 먹은 경험치가 요구량을 넘으면 레벨업 (연속 레벨업 대응을 위해 while문 처리)
	while (CurrentEXP >= MaxEXP)
	{
		LevelUp();
	}

	// 밀린 레벨업이 있고 지금 메뉴가 안 떠 있으면 강화 메뉴 열기 (여러 번 레벨업해도 메뉴는 하나씩)
	if (PendingLevelUps > 0 && !bChoosingUpgrade)
	{
		OpenUpgradeMenu();
	}
}

void AMainCharacter::LevelUp()
{
	CurrentEXP -= MaxEXP;
	CurrentLevel++;

	// 레벨당 요구 경험치 수치 30% 증가
	MaxEXP = FMath::RoundToFloat(MaxEXP * 1.3f);

	// 바로 메뉴를 띄우지 않고 밀린 레벨업 수만 올려둡니다. (메뉴는 GainExp에서 한 번에 하나씩)
	PendingLevelUps++;
}

void AMainCharacter::OpenUpgradeMenu()
{
	// 고를 스킬 하나도 없으면(테이블 미지정, 모든 스킬 최대 레벨 등) 메뉴 없이 그냥 진행
	TArray<FName> RowNames;
	TArray<FMSUpgradeData> Rows;
	GetUpgradeChoices(RowNames, Rows);
	if (RowNames.Num() == 0)
	{
		bStartingChoice = false;
		PendingLevelUps = 0;
		return;
	}

	bChoosingUpgrade = true;

	// 게임을 일시정지하고 마우스 커서를 켜서 스킬를 클릭할 수 있게 합니다.
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	if (ALabyrinthPlayerController* PC = Cast<ALabyrinthPlayerController>(GetController()))
	{
		PC->SetUIInputMode(true);
	}

	// 블루프린트 강화 메뉴 신호
	OnLevelUpBlueprintEvent();
}

void AMainCharacter::OnLevelUpBlueprintEvent_Implementation()
{
	
	TArray<FName> RowNames;
	TArray<FMSUpgradeData> Rows;
	GetUpgradeChoices(RowNames, Rows);
	ApplyUpgrade(RowNames.Num() > 0 ? RowNames[0] : NAME_None);
}

void AMainCharacter::GetUpgradeChoices(TArray<FName>& OutRowNames, TArray<FMSUpgradeData>& OutRows) const
{
	OutRowNames.Reset();
	OutRows.Reset();
	if (!UpgradeTable) return;

	// 1. 후보 고르기
	TArray<FName> Candidates;
	for (const FName& RowName : UpgradeTable->GetRowNames())
	{
		const FMSUpgradeData* Data = UpgradeTable->FindRow<FMSUpgradeData>(RowName, TEXT("GetUpgradeChoices"));
		if (!Data) continue;

		const bool bIsSkill = (Data->UpgradeType == EMSUpgradeType::AttackSkill);

		// 게임 시작 선택이면 공격 스킬만
		if (bStartingChoice && !bIsSkill) continue;

		// 능력치 카드인데 UpgradeID가 비어 있으면 효과가 없으므로 제외 (빈 행 등)
		if (!bIsSkill && Data->UpgradeID.IsNone()) continue;

		// 최대 레벨에 도달한 스킬은 제외
		if (bIsSkill && GetSkillLevel(RowName) >= Data->MaxLevel) continue;

		Candidates.Add(RowName);
	}

	// 2. 섞기
	for (int32 i = Candidates.Num() - 1; i > 0; --i)
	{
		Candidates.Swap(i, FMath::RandRange(0, i));
	}

	// 3. 앞에서 최대 3장
	for (int32 i = 0; i < Candidates.Num() && i < 3; ++i)
	{
		OutRowNames.Add(Candidates[i]);
		OutRows.Add(*UpgradeTable->FindRow<FMSUpgradeData>(Candidates[i], TEXT("GetUpgradeChoices")));
	}
}

int32 AMainCharacter::GetSkillLevel(FName RowName) const
{
	if (UMSSkillComponent* const* Found = OwnedSkills.Find(RowName))
	{
		return (*Found)->GetSkillLevel();
	}
	return 0;
}

void AMainCharacter::ApplyUpgrade(FName RowName)
{
	// 메뉴가 안 떠 있는데 호출되면 무시 (버튼 연타 등)
	if (!bChoosingUpgrade) return;

	const FMSUpgradeData* Data = UpgradeTable ? UpgradeTable->FindRow<FMSUpgradeData>(RowName, TEXT("ApplyUpgrade")) : nullptr;
	if (Data)
	{
		if (Data->UpgradeType == EMSUpgradeType::Stat)
		{
			// --- 능력치 카드: UpgradeID로 구분 ---
			if (Data->UpgradeID == "Stat_Damage")
			{
				DamageMultiplier += Data->Value; // 0.2면 데미지 +20%
			}
			else if (Data->UpgradeID == "Stat_MaxHP")
			{
				MaxHP += Data->Value;     // 최대 체력 증가
				CurrentHP += Data->Value; // 늘어난 만큼 현재 체력도 회복
			}
			else if (Data->UpgradeID == "Stat_MoveSpeed")
			{
				GetCharacterMovement()->MaxWalkSpeed *= (1.0f + Data->Value); // 0.1이면 +10%
			}
		}
		else
		{
			// --- 공격 스킬 카드: 있으면 레벨업, 없으면 획득 ---
			if (UMSSkillComponent** Found = OwnedSkills.Find(RowName))
			{
				(*Found)->LevelUpSkill();
			}
			else
			{
				// 스킬 컴포넌트를 새로 만들어 캐릭터에 붙입니다. (RegisterComponent 때 BeginPlay → 발사 타이머 시작)
				UMSSkillComponent* NewSkill = NewObject<UMSSkillComponent>(this);
				NewSkill->InitSkill(*Data);
				NewSkill->RegisterComponent();
				AddInstanceComponent(NewSkill);
				OwnedSkills.Add(RowName, NewSkill);
			}
		}
	}

	// 이번 선택 끝
	bChoosingUpgrade = false;
	if (bStartingChoice)
	{
		bStartingChoice = false;
	}
	else
	{
		PendingLevelUps = FMath::Max(0, PendingLevelUps - 1);
	}

	// 밀린 레벨업이 남아 있으면 바로 다음 메뉴
	if (PendingLevelUps > 0)
	{
		OpenUpgradeMenu();
		return;
	}

	// 게임 재개 + 마우스 커서 끄기
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	if (ALabyrinthPlayerController* PC = Cast<ALabyrinthPlayerController>(GetController()))
	{
		PC->SetGameInputMode();
	}
}

void AMainCharacter::Die()
{
	// 🔄 ALabyrinthGameModeBase로 변경하여 플레이어가 죽었다고 알립니다.
	ALabyrinthGameModeBase* GM = Cast<ALabyrinthGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->OnPlayerDied();
	}
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "LabyrinthGameModeBase.h"
#include "MainCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class UMSSkillComponent;


UCLASS()
class LABYRINTH_API AMainCharacter : public ACharacter
{
    GENERATED_BODY()

    /** 위에서 내려다보는 고정 카메라 붐 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

public:
    // Sets default values for this character's properties
    AMainCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // 컨트롤러가 빙의될 때 매핑 컨텍스트 등록
    virtual void NotifyControllerChanged() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;


    // ==========================================
        // ⭐ [매직 서바이벌용 핵심 시스템 변수 및 함수] ⭐
        // ==========================================
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
    float CurrentHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxHP = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
    int32 CurrentLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
    float CurrentEXP = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxEXP = 100.0f;

    UFUNCTION(BlueprintCallable, Category = "Survival|Gameplay")
    void TakeSurvivalDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival|Gameplay")
    void GainExp(float Amount);

    // ==========================================
    // ⭐ [강화 시스템] ⭐
    

    // 강화 목록
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival|Upgrade")
    UDataTable* UpgradeTable;

    // 모든 공격 스킬의 데미지 배율 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Upgrade")
    float DamageMultiplier = 1.0f;

    // [BP 레벨업 메뉴에서 호출]최대 3장 (행 이름 + 행 데이터, 같은 순서)
    //  - 게임 시작 선택: 공격 스킬만
    //  - 레벨업: 능력치 + 공격 스킬 (최대 레벨 스킬은 제외)
    //  ※ 랜덤이라 BlueprintPure=false로 실행 핀을 둠 (Pure면 값을 꺼낼 때마다 다시 섞여서 카드가 꼬임)
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Survival|Upgrade")
    void GetUpgradeChoices(TArray<FName>& OutRowNames, TArray<FMSUpgradeData>& OutRows) const;

    // [BP  클릭에서 호출] 고른 카드 효과 적용 → 밀린 레벨업이 있으면 다음 메뉴, 없으면 게임 재개
    UFUNCTION(BlueprintCallable, Category = "Survival|Upgrade")
    void ApplyUpgrade(FName RowName);

    // [BP  표시용] 이 스킬의 현재 레벨 (0 = 아직 없음 → "획득" , 1 이상 → "Lv.N+1" )
    UFUNCTION(BlueprintPure, Category = "Survival|Upgrade")
    int32 GetSkillLevel(FName RowName) const;

    // [BP 메뉴 표시용] 지금 메뉴가 게임 시작 스킬 선택인지 (제목을 "시작 스킬 선택"으로 바꿀 때 등)
    UFUNCTION(BlueprintPure, Category = "Survival|Upgrade")
    bool IsStartingChoice() const { return bStartingChoice; }


    // 카메라 설정 (에디터에서 조절 가능)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    float CameraPitch = -85.0f;   // -90이면 완전 수직 탑다운

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    float CameraDistance = 1200.0f;

protected:

    //움직임
    // 에디터에서 IA_Move를 지정할 변수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    // 실제 캐릭터를 움직이는 함수
    void Move(const FInputActionValue& Value);

    // 컴포넌트와 입력 액션을 연결하는 상속 함수
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
    // 강화 메뉴를 띄우라는 신호 (게임은 이미 일시정지 + 마우스 커서 표시된 상태)
    // BP_MainCharacter에서 이 이벤트를 구현해 WBP_LevelUpMenu를 띄우면 됩니다.
    // BP에서 구현하지 않으면 C++ 기본 동작(첫 번째 카드 자동 선택)으로 게임이 멈추지 않고 진행됩니다.
    UFUNCTION(BlueprintNativeEvent, Category = "Survival|UI")
    void OnLevelUpBlueprintEvent();
    virtual void OnLevelUpBlueprintEvent_Implementation();

private:
    void LevelUp();
    void Die();

    // 일시정지 + UI 입력 모드로 바꾸고 강화 메뉴 이벤트 호출 (고를 카드가 없으면 그냥 진행)
    void OpenUpgradeMenu();

    // 가지고 있는 공격 스킬 (데이터 테이블 행 이름 → 스킬 컴포넌트)
    UPROPERTY()
    TMap<FName, UMSSkillComponent*> OwnedSkills;

    // 경험치를 한 번에 많이 먹었을 때 밀린 레벨업 수 (메뉴는 하나씩 차례대로)
    int32 PendingLevelUps = 0;

    // 지금 강화 메뉴가 떠 있는지 (중복 메뉴, 버튼 연타 방지)
    bool bChoosingUpgrade = false;

    // 지금 메뉴가 게임 시작 스킬 선택인지
    bool bStartingChoice = false;
};
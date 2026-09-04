// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MainCharacter.generated.h"

class UInputAction;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;


UCLASS()
class LABYRINTH_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

public:
	// Sets default values for this character's properties
	AMainCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
public:
    // 능력치 관련
    
    //레벨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
    int32 PlayerLevel = 1;

    //공격력 나중에 컨셉애따라 변경
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
    int32 AttackPoint;

    //체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
    float MaxHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
    float CurrentHP;

    //경험치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
    float MaxEXP;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStats")
    float CurrentEXP;

protected:
    
    //움직임 
    // 에디터에서 IA_Move를 지정할 변수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;


    // 실제 캐릭터를 움직이는 함수
    void Move(const FInputActionValue& Value);

    void Look(const FInputActionValue& Value);

    // 컴포넌트와 입력 액션을 연결하는 상속 함수
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
   
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationYaw = false;

	// 2. 이동 방향으로 캐릭터 몸통이 자동으로 고개를 돌리도록 설정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // 회전 속도

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 입력 컴포넌트를 향상된 입력 컴포넌트로 캐스팅 후 Move 함수 바인딩
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacter::Move);
        }
    }
}
// 움직임
void AMainCharacter::Move(const FInputActionValue& Value)
{
    // IA_Move에서 설정한 Vector2D 값 가져오기 (W,S,A,D 신호)
    FVector2D MovementVector = Value.Get<FVector2D>();
 
    if (Controller != nullptr)
    {
         const FRotator AbsoluteRotation(0.0f, 0.0f, 0.0f);

        // 절대 전방(X축)과 절대 우측(Y축) 방향 벡터 구하기
        const FVector ForwardDirection = FRotationMatrix(AbsoluteRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(AbsoluteRotation).GetUnitAxis(EAxis::Y);

        // 실제 이동 입력 적용 (Add Movement Input 노드와 100% 동일)
        AddMovementInput(ForwardDirection, MovementVector.Y); // W, S 키 처리
        AddMovementInput(RightDirection, MovementVector.X);   // A, D 키 처리
    }
}
void AMainCharacter::Look(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

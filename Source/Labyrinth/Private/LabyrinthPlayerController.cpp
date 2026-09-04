// Fill out your copyright notice in the Description page of Project Settings.


#include "LabyrinthPlayerController.h"
#include "EnhancedInputSubsystems.h"

void ALabyrinthPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 마우스 커서 활성화 (탑다운 필수)
    bShowMouseCursor = true;
    // 2. 시작하자마자 화면 클릭 필요 없이 조작되도록 인풋 모드 설정 (핵심)
    FInputModeGameAndUI InputModeData;
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputModeData.SetHideCursorDuringCapture(false); // 마우스 클릭해도 커서 안 숨기기

    SetInputMode(InputModeData);

    // 향상된 입력 시스템에 IMC 등록
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
}
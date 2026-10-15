// Fill out your copyright notice in the Description page of Project Settings.


#include "LabyrinthPlayerController.h"

ALabyrinthPlayerController::ALabyrinthPlayerController()
{
	// 게임 시작 직후에는 마우스 조준이 필요 없으므로 커서를 숨깁니다.
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void ALabyrinthPlayerController::SetUIInputMode(bool bEnableMouse)
{
	bShowMouseCursor = bEnableMouse;
	bEnableClickEvents = bEnableMouse;
	bEnableMouseOverEvents = bEnableMouse;

	if (bEnableMouse)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}

void ALabyrinthPlayerController::SetGameInputMode()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}
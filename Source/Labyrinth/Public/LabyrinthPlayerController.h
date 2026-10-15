// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LabyrinthPlayerController.generated.h"

UCLASS()
class LABYRINTH_API ALabyrinthPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALabyrinthPlayerController();

	// --- [ UI 및 마우스 제어 핵심 함수 ] ---
	UFUNCTION(BlueprintCallable, Category = "Survival|UI")
	void SetUIInputMode(bool bEnableMouse);

	UFUNCTION(BlueprintCallable, Category = "Survival|UI")
	void SetGameInputMode();
};
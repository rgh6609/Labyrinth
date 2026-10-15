// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "LabyrinthCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class ALabyrinthCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	ALabyrinthCharacter();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

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

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Survival|UI")
	void OnLevelUpBlueprintEvent();

private:
	void LevelUp();
	void Die();
};
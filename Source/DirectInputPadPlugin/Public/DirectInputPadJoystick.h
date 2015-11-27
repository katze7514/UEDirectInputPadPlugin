#pragma once

#include "DirectInputPadState.h"

#include "DirectInputPadJoystick.generated.h"

class FDirectInputJoystick;

// BP公開用DIPadクラス
// BPからは、このクラスを介して操作する
UCLASS()
class UDirectInputPadJoystick : public UObject
{
	GENERATED_BODY()

public:
	// DIKeyに設定されているXIKeyを取得する。何も設定されてないときは、XIGamePad_ENDが返る
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	EXInputPadKeyNames	GetKeyMap(EDirectInputPadKeyNames DIKey);

	// DIKeyをXIKeyとして設定する。XIKeyにXIGamePad_ENDを指定するとKeyMapを削除する
	UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	void				SetKeyMap(EDirectInputPadKeyNames DIKey, EXInputPadKeyNames XIKey);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	bool				IsAxisReverse(EDirectInputPadKeyNames DIAxis);

	UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	void				SetAxisReverse(EDirectInputPadKeyNames DIAxis, bool bReverse);

public:
	void SetJoysticks(const TWeakPtr<FDirectInputJoystick>& Joystick);

private:
	TWeakPtr<FDirectInputJoystick> Joystick_;
};


// この関数を使ってUDirectInputPadJoystickを取得する
UCLASS()
class UDirectInputPadFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,BlueprintPure,Category="DirectInputPad")
	static int32 GetXInputPadNum();

	UFUNCTION(BlueprintCallable,BlueprintPure,Category="DirectInputPad")
	static int32 GetDirectInputPadNum();


	UFUNCTION(BlueprintCallable,BlueprintPure,Category="DirectInputPad")
	static UDirectInputPadJoystick* GetDirectInputPadJoystick(int32 PlayerIndex);

public:
	static void InitDirectInputPadJoystickLibrary();
	static void FinDirectInputPadJoystickLibrary();
};

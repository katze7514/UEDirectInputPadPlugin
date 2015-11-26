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
	// DIKeyに設定されているXIKeyを取得する。何も設定されてないときは、XIGamePad_END
	UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	EXInputPadKeyNames	GetKeyMap(EDirectInputPadKeyNames DIKey);

	// DIKeyを、XIKeyとして設定する
	UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	void				SetKeyMap(EDirectInputPadKeyNames DIKey, EXInputPadKeyNames XIKey);

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
	// 
	UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	static UDirectInputPadJoystick* GetDirectInputPadJoystick(int32 PlayerID);

public:
	static void InitDirectInputPadJoystickLibrary();
	static void FinDirectInputPadJoystickLibrary();
};

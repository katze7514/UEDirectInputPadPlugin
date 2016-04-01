#pragma once

#include "DirectInputPadState.h"

#include "DirectInputPadJoystick.generated.h"

class FDirectInputJoystick;

USTRUCT(BlueprintType)
struct DIRECTINPUTPADPLUGIN_API FDIGamePadKeyState
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDirectInputPadKeyNames> KeyName;

	// KeyNameが
	//  軸の時は、軸の値
	//  ボタンの時、1はPush、-1はRelease
	UPROPERTY(BlueprintReadOnly)
	float Value;

	FDIGamePadKeyState():KeyName(DIGamePad_END),Value(0.f){}
};

// BP公開用DIPadクラス
// BPからは、このクラスを介して操作する
UCLASS(BlueprintType)
class DIRECTINPUTPADPLUGIN_API UDirectInputPadJoystick : public UObject
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

	// 表示名を取得する。同じパッドだと同じ名前になる可能性があるので、識別には使わないこと
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	FString				GetProductName();

	// GUID(文字列)を取得する。パッドの識別に使える
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	FString				GetGUID();
	
	// 現在、状態の変化があったキーの情報を得る
	// 最初に見つかった一つが取得できる。軸(XYZ軸、XYZ回転)＞ボタン(1→32)の優先度でチェックする
	// POVは含まれない
	// bRealがtrueの時は、反転フラグなどの設定を無視した実データで判定される
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	FDIGamePadKeyState				GetChangedKeyState(bool bReal=false);

	// 現在、状態の変化があったキーの情報をすべて得る。POVは含まれない
	// bRealがtrueの時は、反転フラグなどの設定を無視した実データで判定される
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	TArray<FDIGamePadKeyState>		GetAllChangedKeyState(bool bReal=false);

	// 入力データをクリアする
	UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	void				ClearInput();

public:
	void SetJoysticks(const TWeakPtr<FDirectInputJoystick>& Joystick);

private:
	TWeakPtr<FDirectInputJoystick> Joystick_;
};


// この関数を使ってUDirectInputPadJoystickを取得する
UCLASS()
class DIRECTINPUTPADPLUGIN_API UDirectInputPadFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,BlueprintPure,Category="DirectInputPad")
	static bool IsInitilizedDirectInputPadPlugin();

	// 接続されているXInputパッドの数。取得できなかった時は-1が返る
	UFUNCTION(BlueprintCallable,BlueprintPure,Category="DirectInputPad")
	static int32 GetXInputPadNum();

	// 接続されているDirectInputパッドの数。取得できなかった時は-1が返る
	UFUNCTION(BlueprintCallable,BlueprintPure,Category="DirectInputPad")
	static int32 GetDirectInputPadNum();

	UFUNCTION(BlueprintCallable,BlueprintPure,Category="DirectInputPad")
	static UDirectInputPadJoystick* GetDirectInputPadJoystick(int32 PlayerIndex);

public:
	static void InitDirectInputPadJoystickLibrary();
	static void FinDirectInputPadJoystickLibrary();
};

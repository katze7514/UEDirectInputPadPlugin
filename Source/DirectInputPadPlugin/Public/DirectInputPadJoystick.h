#pragma once

#include "DirectInputPadState.h"

#include "DirectInputPadJoystick.generated.h"

class FXInputJoystickEmu;

USTRUCT(BlueprintType)
struct DIRECTINPUTPADPLUGIN_API FDIGamePadKeyState
{
	GENERATED_BODY()

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
	// XIKeyに設定されているDIKeyMapを取得する
	// 何も設定されてないときは、eDIKey_がDIGamePad_ENDになっている
	// eDIKeyが軸の場合、0に正が、1に負方向の割り当てば入ってくる
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	TArray<FDIKeyMapInfo> GetKeyMap(EXInputPadKeyNames eDIKey);

	// XIKeyとしてふるまうDIKeyを設定する。DIGamePad_Endを指定するとマップを削除する
	// アナログにボタンを設定した場合は、ボタンが押されている時に値を返す
	//  bNegativeがfalseなら1.0を、trueなら-1.0を返す
	// ボタンにアナログを設定した場合は、アナログの入力が閾値以上の時、ボタンが押されことになる
	//  bNegativeがfalseなら正方向、bNegativeがtrueなら負方向の値で判定される
	// アナログにアナログ、ボタンにボタンの時はbNegativeフラグは無視される
	UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	void				SetKeyMap(EXInputPadKeyNames XIKey, EDirectInputPadKeyNames DIKey, bool bNegative=false);

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
	
	// 状態が変化したキーがあるかどうか
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	bool							IsChagnedKeyState()const;

	// 現在、状態の変化があったキーの情報を得る
	// 最初に見つかった一つが取得できる
	// の優先度でチェックする
	// bRealがtrueの時は、反転フラグなどの設定を無視した実データで判定される
	// bBtnが
	//    trueの時は、ボタン(1→32)＞POV(上右下左)＞軸(XYZ軸、XYZ回転)
	//    falseの時は、軸(XYZ軸、XYZ回転)＞ボタン(1→32)＞POV(上右下左)
	// の順番でチェックする
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	FDIGamePadKeyState				GetChangedKeyState(bool bReal=true, bool bBtn=false);

	// 現在、状態の変化があったキーの情報をすべて得る
	// bRealがtrueの時は、反転フラグなどの設定を無視した実データで判定される
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	TArray<FDIGamePadKeyState>		GetAllChangedKeyState(bool bReal=false);

	// 入力データをクリアする
	UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	void				ClearInput();

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="DirectInputPad")
	int32	GetPlayerIndex()const;

	//UFUNCTION(BlueprintCallable, Category="DirectInputPad")
	//void	SetPlayerIndex(int32 PlayerIndex);

public:
	void SetJoysticks(const TWeakPtr<FXInputJoystickEmu>& Joystick);

private:
	TWeakPtr<FXInputJoystickEmu> Joystick_;
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

	UFUNCTION(BlueprintCallable,Category="DirectInputPad")
	static bool SaveDirectInputPadKeyMap(const FString& SlotName, int32 UserIndex);

	UFUNCTION(BlueprintCallable,Category="DirectInputPad")
	static bool LoadDirectInputPadKeyMap(const FString& SlotName, int32 UserIndex);

public:
	static void InitDirectInputPadJoystickLibrary();
	static void FinDirectInputPadJoystickLibrary();
};

#pragma once

#include <functional>
#include "DirectInputPadState.h"

class FDirectInputJoystick;

struct AnalogDelegate
{
	std::function<float()>	Axis_;

	// Axisにボタンが設定されていた場合、
	// 負の方向をチェックするのに使う
	std::function<float()>	AxisNegative_;

	FDIKeyMapInfo DIKeyAxis_;
	FDIKeyMapInfo DIKeyAxisNegative_;
};

struct BtnDelegate
{
	std::function<bool()>	IsPress_;
	std::function<bool()>	IsPush_;
	std::function<bool()>	IsRelease_;

	FDIKeyMapInfo DIKey_;
};

//! DirectInputPadをXInputとしてエミュレーションするクラス
class FXInputJoystickEmu
{
public:
	void Init(TSharedRef<FDirectInputJoystick> DIPad);

	//! 状態に合わせてEventを発生させる
	void Event(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler);

private:
	void EventAnalog(FName XIName, const AnalogDelegate& Delegate, const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler);
	void EventButton(FName XIName, const BtnDelegate& Delegate, const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler);

public:
	//! キーマップを設定する
	void SetKeyMap(EXInputPadKeyNames eXIKey, FDIKeyMapInfo DIKeyInfo);

private:
	//! アナログマップを設定する。eDIKeyがボタンの場合、bNegativeに従った場所に設定される
	//! eXIKeyがボタンの場合は、何もおこらない
	void SetAnalogMap(EXInputPadKeyNames eXIKey, FDIKeyMapInfo DIKeyInfo);
	//! ボタンマップを設定する。eDIKeyが軸の場合、bNegativeに従った方向の値が使われる
	//! eXIKeyがアナログの場合、アナログのボタン動作部分に設定される
	void SetButtonMap(EXInputPadKeyNames eXIKey, FDIKeyMapInfo DIKeyInfo);

public:
	TArray<FDIKeyMapInfo> GetKeyMap(EXInputPadKeyNames eXIKey);

private:
	//! eXIKeyとして設定されているキーマップ情報を取り出す
	TArray<FDIKeyMapInfo>	GetAnalogDIKeyMapInfo(EXInputPadKeyNames eXIKey)const;
	FDIKeyMapInfo			GetButtonDIKeyMapInfo(EXInputPadKeyNames eXIKey)const;

public:
	TSharedPtr<FDirectInputJoystick> GetDIPad()const{ return DIPad_; }

private:
	void SetAnalogMapAnalog(EDirectInputPadKeyNames eDIKey, AnalogDelegate& Analog, BtnDelegate& Pos, BtnDelegate& Neg);
	void SetAnalogMapBtn(EDirectInputPadKeyNames eDIKey, AnalogDelegate& Analog, BtnDelegate& Btn, bool bNegative=false);

	// bNegativeがtrueの時、ボタンを使ったdelegateが負の値を返すようになる
	std::function<float()>			CreateAnalogDelegate(EDirectInputPadKeyNames eDIKey, bool bNegative=false);
	// 0:Press 1:Push 2:Release　bNegativeがtrueの時、軸を使ったdelegateは負の方向を使って判定する
	TArray<std::function<bool()>>	CreateButtonDelegate(EDirectInputPadKeyNames eDIKey, bool bNegative=false);

private:
	// 対応するDirectInputPad
	TSharedPtr<FDirectInputJoystick> DIPad_;

	// デリゲート群
	AnalogDelegate	LeftAnalogX_;
	AnalogDelegate	LeftAnalogY_;
	AnalogDelegate	RightAnalogX_;
	AnalogDelegate	RightAnalogY_;
	AnalogDelegate	LeftTriggerAnalog_;  // 0.0~1.0
	AnalogDelegate	RightTriggerAnalog_; // 0.0~1.0

	BtnDelegate		LeftStickUp_;
	BtnDelegate		LeftStickDown_;
	BtnDelegate		LeftStickRight_;
	BtnDelegate		LeftStickLeft_;

	BtnDelegate		RightStickUp_;
	BtnDelegate		RightStickDown_;
	BtnDelegate		RightStickRight_;
	BtnDelegate		RightStickLeft_;

	BtnDelegate		BtnA_;
	BtnDelegate		BtnB_;
	BtnDelegate		BtnX_;
	BtnDelegate		BtnY_;

	BtnDelegate		BtnLB_;
	BtnDelegate		BtnRB_;
	BtnDelegate		LeftTrigger_;
	BtnDelegate		RightTrigger_;

	BtnDelegate		BtnBack_;
	BtnDelegate		BtnStart_;

	BtnDelegate		BtnLStick_;
	BtnDelegate		BtnRStick_;

	BtnDelegate		DPadUp_;
	BtnDelegate		DPadDown_;
	BtnDelegate		DPadRight_;
	BtnDelegate		DPadLeft_;
};

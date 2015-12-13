#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputPadState.h"

#include "DirectInputDriver.h"
#include "DirectInputJoystick.h"

DEFINE_LOG_CATEGORY_STATIC(DirectInputPadPlugin, Log, All)

#include "AllowWindowsPlatformTypes.h"

//////////////////////////////////////
// FDirectInputJoystick
/////////////////////////////////////
const int32_t FDirectInputJoystick::MAX_AXIS_VALUE =  1000;

bool FDirectInputJoystick::Init(const DIDEVICEINSTANCE& joyins, FDirectInputDriver& adapter, HWND hWnd, bool bBackGround)
{
	if(pDevice_) return true;

	auto driver = adapter.driver();

	HRESULT r = driver->CreateDevice(joyins.guidInstance, &pDevice_, NULL);
	if(r!=DI_OK)
	{
		UE_LOG(DirectInputPadPlugin, Error, TEXT("Joystick CreateDevice fail. : %x"),r);
		return false;
	}

	r = pDevice_->SetDataFormat(&c_dfDIJoystick);
	if(r!=DI_OK)
	{
		UE_LOG(DirectInputPadPlugin, Error, TEXT("Joystick SetDataformat fail. : %x"),r);
		Fin();
		return false;
	}

	// CooperativeLevelフラグ決定
	DWORD flags = 0;
	
	if(bBackGround) // ウインドウが非アクティブでも取得される
		flags = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE;
	else
		flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;

	r = pDevice_->SetCooperativeLevel(hWnd, flags);
	if(r!=DI_OK)
	{
		UE_LOG(DirectInputPadPlugin, Error, TEXT("Joystick SetCooperativeLevel fail. : %x"), r);
		Fin();
		return false;
	}

	// 軸の設定

	// 絶対軸モードに設定
	DIPROPDWORD diprop;
	diprop.diph.dwSize			= sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize	= sizeof(DIPROPHEADER);
	diprop.diph.dwHow			= DIPH_DEVICE;
	diprop.diph.dwObj			= 0;
	diprop.dwData				= DIPROPAXISMODE_ABS;

	r = pDevice_->SetProperty(DIPROP_AXISMODE, &diprop.diph);
	if(r!=DI_OK)
	{
		UE_LOG(DirectInputPadPlugin, Error, TEXT("Joystick AxisMode Setup fail. : %x"), r);
		Fin();
		return false;
	}

	// 軸の範囲を設定
	std::tuple<HRESULT, LPDIRECTINPUTDEVICE8> axisResult(DI_OK, pDevice_);
	pDevice_->EnumObjects(&FDirectInputJoystick::OnEnumAxis, reinterpret_cast<void*>(&axisResult), DIDFT_AXIS);

	if(std::get<0>(axisResult)!=DI_OK)
	{
		Fin();
		return false;
	}

	// デフォルトのマップを設定
	InitDefaultMap();

	AxisReverseFlagMap_.SetNumUninitialized(DIGamePad_ROT_Z+1);
	for(uint32 i=DIGamePad_AXIS_X; i<=DIGamePad_ROT_Z; ++i)
		AxisReverseFlagMap_[i] = false;

//	const string sFlag = (flags&DISCL_BACKGROUND)>0 ? "BACKGROUND" : "FOREGROUND";
	UE_LOG(DirectInputPadPlugin, Log, TEXT("DirectInput Joystick Create Success. : %s"), joyins.tszProductName);

	return true;
}

void FDirectInputJoystick::InitDefaultMap()
{
	JoystickMap_.SetNumUninitialized(DIGamePad_END);
	
	JoystickMap_[DIGamePad_AXIS_X]		= FGamepadKeyNames::LeftAnalogX;
	JoystickMap_[DIGamePad_AXIS_Y]		= FGamepadKeyNames::LeftAnalogY;
	JoystickMap_[DIGamePad_AXIS_Z]		= FGamepadKeyNames::RightAnalogX;
	JoystickMap_[DIGamePad_ROT_X]		= FName();
	JoystickMap_[DIGamePad_ROT_Y]		= FName();
	JoystickMap_[DIGamePad_ROT_Z]		= FGamepadKeyNames::RightAnalogY;

	JoystickMap_[DIGamePad_POV]			= FName(); // POVは入力を取る段階で分解する

	JoystickMap_[DIGamePad_Button1]		= FGamepadKeyNames::FaceButtonBottom;		// A
	JoystickMap_[DIGamePad_Button2]		= FGamepadKeyNames::FaceButtonRight;		// B
	JoystickMap_[DIGamePad_Button3]		= FGamepadKeyNames::FaceButtonLeft;			// X
	JoystickMap_[DIGamePad_Button4]		= FGamepadKeyNames::FaceButtonTop;			// Y
	JoystickMap_[DIGamePad_Button5]		= FGamepadKeyNames::LeftShoulder;			// L1
	JoystickMap_[DIGamePad_Button6]		= FGamepadKeyNames::RightShoulder;			// R1
	JoystickMap_[DIGamePad_Button7]		= FGamepadKeyNames::LeftTriggerThreshold;	// L2
	JoystickMap_[DIGamePad_Button8]		= FGamepadKeyNames::RightTriggerThreshold;	// R2
	JoystickMap_[DIGamePad_Button9]		= FGamepadKeyNames::SpecialLeft;			// SELECT
	JoystickMap_[DIGamePad_Button10]	= FGamepadKeyNames::SpecialRight;			// START
	JoystickMap_[DIGamePad_Button11]	= FGamepadKeyNames::LeftThumb;				// Lスティック押し
	JoystickMap_[DIGamePad_Button12]	= FGamepadKeyNames::RightThumb;				// Rスティック押し

	// 残りのボタンは、とりあえず、空
	for(uint8 i=DIGamePad_Button13; i<=DIGamePad_Button32; ++i)
	{	JoystickMap_[i] = FName();	}

	SetDelegateLeftAnalogX(DIGamePad_AXIS_X);
	SetDelegateLeftAnalogY(DIGamePad_AXIS_Y);
	SetDelegateRightAnalogX(DIGamePad_AXIS_Z);
	SetDelegateRightAnalogY(DIGamePad_ROT_Z);
}

void FDirectInputJoystick::Fin()
{
	if(pDevice_)
	{
		if(bAcquire_) pDevice_->Unacquire();
		pDevice_->Release();
		pDevice_=nullptr;
	}
}

BOOL CALLBACK FDirectInputJoystick::OnEnumAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	std::tuple<HRESULT, LPDIRECTINPUTDEVICE8>* pAxisResult = reinterpret_cast<std::tuple<HRESULT, LPDIRECTINPUTDEVICE8>*>(pvRef);

	DIPROPRANGE diprg;
	diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
	diprg.diph.dwHow        = DIPH_BYID; 
	diprg.diph.dwObj        = lpddoi->dwType;
	diprg.lMin              = -MAX_AXIS_VALUE;
	diprg.lMax              =  MAX_AXIS_VALUE;

	std::get<0>(*pAxisResult) = std::get<1>(*pAxisResult)->SetProperty(DIPROP_RANGE, &diprg.diph);
	if(std::get<0>(*pAxisResult)!=DI_OK) return DIENUM_STOP;

	//if(!check_hresult(pAxisResult->get<0>(), "[FDirectInputJoystick]" + string(lpddoi->tszName) + "の範囲を設定できませんでした："))
	//	return DIENUM_STOP;

	return DIENUM_CONTINUE;
}

bool FDirectInputJoystick::Input()
{
	if(!pDevice_) return false;

	HRESULT r;
	// デバイス獲得してなかったら獲得する
	if(!bAcquire_)
	{
		r = pDevice_->Acquire();
		if(FAILED(r))
		{// 獲得できなかたっら次の機会へ
			//logger::warnln("[FDirectInputJoystick]Lost: " + to_str(r));
			ClearCurBuf();
			return false;
		}

		bAcquire_=true;
	}

	// 入力ガードが有効だったらPollしない
	if(IsGuard()) return true;

	// 入力データを取得
	nCurIndex_ ^= 1;

	// ポーリング
	r = pDevice_->Poll();
	if(r==DIERR_INPUTLOST)
	{// 獲得権がなかったら1度だけ取得しに行く
		bAcquire_ =false;

		r = pDevice_->Acquire();
		if(FAILED(r))
		{// 獲得できなかたっら次の機会へ
			ClearCurBuf();
			return false;
		}
		else
		{
			bAcquire_ = true;
			// 改めてPoll
			r = pDevice_->Poll();
			if(FAILED(r))
			{
				ClearCurBuf();
				return false;
			}
		}
	}

	// データ取得
	r = pDevice_->GetDeviceState(sizeof(DIJOYSTATE), &joyBuf_[nCurIndex_]);
	if(r==DIERR_INPUTLOST)
	{// 獲得権がなかったら1度だけ取得しに行く
		bAcquire_ =false;

		r = pDevice_->Acquire();
		if(FAILED(r))
		{// 獲得できなかたっら次の機会へ
			ClearCurBuf();
			return false;
		}
		else
		{
			bAcquire_ = true;
			r = pDevice_->GetDeviceState(sizeof(DIJOYSTATE), &joyBuf_[nCurIndex_]);
			if(FAILED(r))
			{
				ClearCurBuf();
				return false;
			}
		}
	}

	return true;
}

/////////////////////////////
// イベント
/////////////////////////////
void FDirectInputJoystick::Event(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
	// JoystickMapをぶん回す

	// 軸チェック
	EventAnalog(MessageHandler, X(), DIGamePad_AXIS_X, EKeysDirectInputPad::DIGamePad_AxisX);
	EventAnalog(MessageHandler, Y(), DIGamePad_AXIS_Y, EKeysDirectInputPad::DIGamePad_AxisY);
	EventAnalog(MessageHandler, Z(), DIGamePad_AXIS_Z, EKeysDirectInputPad::DIGamePad_AxisZ);

	EventAnalog(MessageHandler, RotX(), DIGamePad_ROT_X, EKeysDirectInputPad::DIGamePad_RotX);
	EventAnalog(MessageHandler, RotY(), DIGamePad_ROT_Y, EKeysDirectInputPad::DIGamePad_RotY);
	EventAnalog(MessageHandler, RotZ(), DIGamePad_ROT_Z, EKeysDirectInputPad::DIGamePad_RotZ);

	EventPov(MessageHandler);

	// ボタンチェック
	EventButton(MessageHandler, DIGamePad_Button1, EKeysDirectInputPad::DIGamePad_Button1);
	EventButton(MessageHandler, DIGamePad_Button2, EKeysDirectInputPad::DIGamePad_Button2);
	EventButton(MessageHandler, DIGamePad_Button3, EKeysDirectInputPad::DIGamePad_Button3);
	EventButton(MessageHandler, DIGamePad_Button4, EKeysDirectInputPad::DIGamePad_Button4);
	EventButton(MessageHandler, DIGamePad_Button5, EKeysDirectInputPad::DIGamePad_Button5);
	EventButton(MessageHandler, DIGamePad_Button6, EKeysDirectInputPad::DIGamePad_Button6);
	EventButton(MessageHandler, DIGamePad_Button7, EKeysDirectInputPad::DIGamePad_Button7);
	EventButton(MessageHandler, DIGamePad_Button8, EKeysDirectInputPad::DIGamePad_Button8);
	EventButton(MessageHandler, DIGamePad_Button9, EKeysDirectInputPad::DIGamePad_Button9);
	EventButton(MessageHandler, DIGamePad_Button10, EKeysDirectInputPad::DIGamePad_Button10);
	EventButton(MessageHandler, DIGamePad_Button11, EKeysDirectInputPad::DIGamePad_Button11);
	EventButton(MessageHandler, DIGamePad_Button12, EKeysDirectInputPad::DIGamePad_Button12);
	EventButton(MessageHandler, DIGamePad_Button13, EKeysDirectInputPad::DIGamePad_Button13);
	EventButton(MessageHandler, DIGamePad_Button14, EKeysDirectInputPad::DIGamePad_Button14);
	EventButton(MessageHandler, DIGamePad_Button15, EKeysDirectInputPad::DIGamePad_Button15);
	EventButton(MessageHandler, DIGamePad_Button16, EKeysDirectInputPad::DIGamePad_Button16);
	EventButton(MessageHandler, DIGamePad_Button17, EKeysDirectInputPad::DIGamePad_Button17);
	EventButton(MessageHandler, DIGamePad_Button18, EKeysDirectInputPad::DIGamePad_Button18);
	EventButton(MessageHandler, DIGamePad_Button19, EKeysDirectInputPad::DIGamePad_Button19);
	EventButton(MessageHandler, DIGamePad_Button20, EKeysDirectInputPad::DIGamePad_Button20);
	EventButton(MessageHandler, DIGamePad_Button21, EKeysDirectInputPad::DIGamePad_Button21);
	EventButton(MessageHandler, DIGamePad_Button22, EKeysDirectInputPad::DIGamePad_Button22);
	EventButton(MessageHandler, DIGamePad_Button23, EKeysDirectInputPad::DIGamePad_Button23);
	EventButton(MessageHandler, DIGamePad_Button24, EKeysDirectInputPad::DIGamePad_Button24);
	EventButton(MessageHandler, DIGamePad_Button25, EKeysDirectInputPad::DIGamePad_Button25);
	EventButton(MessageHandler, DIGamePad_Button26, EKeysDirectInputPad::DIGamePad_Button26);
	EventButton(MessageHandler, DIGamePad_Button27, EKeysDirectInputPad::DIGamePad_Button27);
	EventButton(MessageHandler, DIGamePad_Button28, EKeysDirectInputPad::DIGamePad_Button28);
	EventButton(MessageHandler, DIGamePad_Button29, EKeysDirectInputPad::DIGamePad_Button29);
	EventButton(MessageHandler, DIGamePad_Button30, EKeysDirectInputPad::DIGamePad_Button30);
	EventButton(MessageHandler, DIGamePad_Button31, EKeysDirectInputPad::DIGamePad_Button31);
	EventButton(MessageHandler, DIGamePad_Button32, EKeysDirectInputPad::DIGamePad_Button32);
}

void FDirectInputJoystick::EventAnalog(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, float Analog, EDirectInputPadKeyNames ePadName, FKey DIKey)
{
	MessageHandler->OnControllerAnalog(DIKey.GetFName(), GetPlayerIndex(), Analog);

	if(!JoystickMap_[ePadName].IsNone())
	{
		MessageHandler->OnControllerAnalog(JoystickMap_[ePadName], GetPlayerIndex(), Analog);

		// スティックのデジタル入力チェック
		if(JoystickMap_[ePadName]==FGamepadKeyNames::LeftAnalogX)
		{
			if(IsAxisPush(AXIS_RIGHT))
			{	MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::LeftStickRight, GetPlayerIndex(), false);	}
			else if(IsAxisRelease(AXIS_RIGHT))
			{	MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::LeftStickRight, GetPlayerIndex(), false);	}
			else if(IsAxisPush(AXIS_LEFT))
			{	MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::LeftStickLeft, GetPlayerIndex(), false);	}
			else if(IsAxisRelease(AXIS_LEFT))
			{	MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::LeftStickLeft, GetPlayerIndex(), false);	}
		}
		else if(JoystickMap_[ePadName]==FGamepadKeyNames::LeftAnalogY)
		{
			if(IsAxisPush(AXIS_UP))
			{	MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::LeftStickUp, GetPlayerIndex(), false);		}
			else if(IsAxisRelease(AXIS_UP))
			{	MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::LeftStickUp, GetPlayerIndex(), false);	}
			else if(IsAxisPush(AXIS_DOWN))
			{	MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::LeftStickDown, GetPlayerIndex(), false);	}
			else if(IsAxisRelease(AXIS_DOWN))
			{	MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::LeftStickDown, GetPlayerIndex(), false);	}
		}

		if(JoystickMap_[ePadName]==FGamepadKeyNames::RightAnalogX)
		{
			if(IsAxisRightPush(AXIS_RIGHT))
			{	MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::RightStickRight, GetPlayerIndex(), false);	}
			else if(IsAxisRightRelease(AXIS_RIGHT))
			{	MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::RightStickRight, GetPlayerIndex(), false);}
			else if(IsAxisRightPush(AXIS_LEFT))
			{	MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::RightStickLeft, GetPlayerIndex(), false);	}
			else if(IsAxisRightRelease(AXIS_LEFT))
			{	MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::RightStickLeft, GetPlayerIndex(), false);	}
		}
		else if(JoystickMap_[ePadName]==FGamepadKeyNames::RightAnalogY)
		{
			if(IsAxisPush(AXIS_UP))
			{	MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::RightStickUp, GetPlayerIndex(), false);	}
			else if(IsAxisRightRelease(AXIS_UP))
			{	MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::RightStickUp, GetPlayerIndex(), false);	}
			else if(IsAxisRightPush(AXIS_DOWN))
			{	MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::RightStickDown, GetPlayerIndex(), false);	}
			else if(IsAxisRightRelease(AXIS_DOWN))
			{	MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::RightStickDown, GetPlayerIndex(), false);	}
		}
	}
}

void FDirectInputJoystick::EventButton(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, EDirectInputPadKeyNames ePadName, FKey DIKey)
{
	EventButtonPressed(MessageHandler, ePadName, DIKey);
	EventButtonReleased(MessageHandler, ePadName, DIKey);
}

void FDirectInputJoystick::EventButtonPressed(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, EDirectInputPadKeyNames ePadName, FKey DIKey)
{
	if(!IsPush(ePadName-DIGamePad_Button1)) return;

	MessageHandler->OnControllerButtonPressed(DIKey.GetFName(), GetPlayerIndex(), false);
	if(!JoystickMap_[ePadName].IsNone())
		MessageHandler->OnControllerButtonPressed(JoystickMap_[ePadName], GetPlayerIndex(), false);
}

void FDirectInputJoystick::EventButtonReleased(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, EDirectInputPadKeyNames ePadName, FKey DIKey)
{
	if(!IsRelease(ePadName-DIGamePad_Button1)) return;

	MessageHandler->OnControllerButtonReleased(DIKey.GetFName(), GetPlayerIndex(), false);
	if(!JoystickMap_[ePadName].IsNone())
		MessageHandler->OnControllerButtonReleased(JoystickMap_[ePadName], GetPlayerIndex(), false);
}

void FDirectInputJoystick::EventPov(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
	if(IsPush(POV_UP))
	{
		MessageHandler->OnControllerButtonPressed(EKeysDirectInputPad::DIGamePad_POV_Up.GetFName(), GetPlayerIndex(), false);
		MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::DPadUp, GetPlayerIndex(), false);
	}
	else if(IsRelease(POV_UP))
	{
		MessageHandler->OnControllerButtonReleased(EKeysDirectInputPad::DIGamePad_POV_Up.GetFName(), GetPlayerIndex(), false);
		MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::DPadUp, GetPlayerIndex(), false);
	}
	else if(IsPush(POV_DOWN))
	{
		MessageHandler->OnControllerButtonPressed(EKeysDirectInputPad::DIGamePad_POV_Down.GetFName(), GetPlayerIndex(), false);
		MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::DPadDown, GetPlayerIndex(), false);
	}
	else if(IsRelease(POV_DOWN))
	{
		MessageHandler->OnControllerButtonReleased(EKeysDirectInputPad::DIGamePad_POV_Down.GetFName(), GetPlayerIndex(), false);
		MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::DPadDown, GetPlayerIndex(), false);
	}

	if(IsPush(POV_RIGHT))
	{
		MessageHandler->OnControllerButtonPressed(EKeysDirectInputPad::DIGamePad_POV_Right.GetFName(), GetPlayerIndex(), false);
		MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::DPadRight, GetPlayerIndex(), false);
	}
	else if(IsRelease(POV_RIGHT))
	{
		MessageHandler->OnControllerButtonReleased(EKeysDirectInputPad::DIGamePad_POV_Right.GetFName(), GetPlayerIndex(), false);
		MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::DPadRight, GetPlayerIndex(), false);
	}
	else if(IsPush(POV_LEFT))
	{
		MessageHandler->OnControllerButtonPressed(EKeysDirectInputPad::DIGamePad_POV_Left.GetFName(), GetPlayerIndex(), false);
		MessageHandler->OnControllerButtonPressed(FGamepadKeyNames::DPadLeft, GetPlayerIndex(), false);
	}
	else if(IsRelease(POV_LEFT))
	{
		MessageHandler->OnControllerButtonReleased(EKeysDirectInputPad::DIGamePad_POV_Left.GetFName(), GetPlayerIndex(), false);
		MessageHandler->OnControllerButtonReleased(FGamepadKeyNames::DPadLeft, GetPlayerIndex(), false);
	}
}

/////////////////////////////
// Config設定
/////////////////////////////
void FDirectInputJoystick::SetGuard(bool bGuard)
{
	bGuard_ = bGuard;
	if(bGuard_) ClearBuf();
}

FName FDirectInputJoystick::GetXIKey(EDirectInputPadKeyNames ePadKey)
{
	if(ePadKey>= EDirectInputPadKeyNames::DIGamePad_END) return FName("");
	return JoystickMap_[ePadKey];
}

void FDirectInputJoystick::SetXIKey(EDirectInputPadKeyNames ePadKey, FName UEKeyName)
{
	//UE_LOG(DirectInputPadPlugin, Log, TEXT("SetXIKey: %d, %s"), (int32)ePadKey, *(UEKeyName.ToString()));

	if(ePadKey >= DIGamePad_END) return;

	if(UEKeyName.IsNone())
	{
		JoystickMap_[ePadKey] = UEKeyName;
		return;
	}

	// 軸は軸にしか設定できない
	if(ePadKey>=DIGamePad_AXIS_X && ePadKey<=DIGamePad_ROT_Z)
	{
		if(!(  UEKeyName == FGamepadKeyNames::LeftAnalogX
			|| UEKeyName == FGamepadKeyNames::LeftAnalogY
			|| UEKeyName == FGamepadKeyNames::RightAnalogX
			|| UEKeyName == FGamepadKeyNames::RightAnalogY
			)) return;

		if(UEKeyName == FGamepadKeyNames::LeftAnalogX)
		{	SetDelegateLeftAnalogX(ePadKey);	}
		else if(UEKeyName == FGamepadKeyNames::LeftAnalogY)
		{	SetDelegateLeftAnalogY(ePadKey);	}
		else if(UEKeyName == FGamepadKeyNames::RightAnalogX)
		{	SetDelegateRightAnalogX(ePadKey);	}
		else if(UEKeyName == FGamepadKeyNames::RightAnalogY)
		{	SetDelegateRightAnalogY(ePadKey);	}
	}

	// ボタンにはボタンしか設定できない
	if(ePadKey>=DIGamePad_Button1 && ePadKey<=DIGamePad_Button32)
	{
		if(!(  UEKeyName == FGamepadKeyNames::FaceButtonBottom
			|| UEKeyName == FGamepadKeyNames::FaceButtonRight
			|| UEKeyName == FGamepadKeyNames::FaceButtonLeft
			|| UEKeyName == FGamepadKeyNames::FaceButtonTop
			|| UEKeyName == FGamepadKeyNames::LeftShoulder
			|| UEKeyName == FGamepadKeyNames::RightShoulder
			|| UEKeyName == FGamepadKeyNames::LeftTriggerThreshold
			|| UEKeyName == FGamepadKeyNames::RightTriggerThreshold
			|| UEKeyName == FGamepadKeyNames::SpecialLeft
			|| UEKeyName == FGamepadKeyNames::SpecialRight
			|| UEKeyName == FGamepadKeyNames::LeftThumb
			|| UEKeyName == FGamepadKeyNames::RightThumb
			))
			return;
	}

	JoystickMap_[ePadKey] = UEKeyName;
}

bool FDirectInputJoystick::IsAxisReverse(EDirectInputPadKeyNames ePadAxis)const
{
	if(!AxisReverseFlagMap_.IsValidIndex(ePadAxis)) return false;
	return AxisReverseFlagMap_[ePadAxis];
}

void FDirectInputJoystick::SetAxisReverse(EDirectInputPadKeyNames ePadAxis, bool bReverse)
{
	if(!AxisReverseFlagMap_.IsValidIndex(ePadAxis)) return;
	AxisReverseFlagMap_[ePadAxis] = bReverse;

}

void FDirectInputJoystick::SetDelegateLeftAnalogX(EDirectInputPadKeyNames ePadKey)
{
	switch(ePadKey)
	{
	case DIGamePad_AXIS_X:
		LeftAnalogX		= &FDirectInputJoystick::X;
		LeftAnalogPrevX = &FDirectInputJoystick::PrevX;
	break;

	case DIGamePad_AXIS_Y:
		LeftAnalogX		= &FDirectInputJoystick::Y;
		LeftAnalogPrevX = &FDirectInputJoystick::PrevY;
	break;

	case DIGamePad_AXIS_Z:
		LeftAnalogX		= &FDirectInputJoystick::Z;
		LeftAnalogPrevX = &FDirectInputJoystick::PrevZ;
	break;

	case DIGamePad_ROT_X:
		LeftAnalogX		= &FDirectInputJoystick::RotX;
		LeftAnalogPrevX = &FDirectInputJoystick::RotPrevX;
	break;

	case DIGamePad_ROT_Y:
		LeftAnalogX		= &FDirectInputJoystick::RotY;
		LeftAnalogPrevX = &FDirectInputJoystick::RotPrevY;
	break;

	case DIGamePad_ROT_Z:
		LeftAnalogX		= &FDirectInputJoystick::RotZ;
		LeftAnalogPrevX = &FDirectInputJoystick::RotPrevZ;
	break;
	}
}

void FDirectInputJoystick::SetDelegateLeftAnalogY(EDirectInputPadKeyNames ePadKey)
{
	switch(ePadKey)
	{
	case DIGamePad_AXIS_X:
		LeftAnalogY		= &FDirectInputJoystick::X;
		LeftAnalogPrevY = &FDirectInputJoystick::PrevX;
	break;

	case DIGamePad_AXIS_Y:
		LeftAnalogY		= &FDirectInputJoystick::Y;
		LeftAnalogPrevY = &FDirectInputJoystick::PrevY;
	break;

	case DIGamePad_AXIS_Z:
		LeftAnalogY		= &FDirectInputJoystick::Z;
		LeftAnalogPrevY = &FDirectInputJoystick::PrevZ;
	break;

	case DIGamePad_ROT_X:
		LeftAnalogY		= &FDirectInputJoystick::RotX;
		LeftAnalogPrevY = &FDirectInputJoystick::RotPrevX;
	break;

	case DIGamePad_ROT_Y:
		LeftAnalogY		= &FDirectInputJoystick::RotY;
		LeftAnalogPrevY = &FDirectInputJoystick::RotPrevY;
	break;

	case DIGamePad_ROT_Z:
		LeftAnalogY		= &FDirectInputJoystick::RotZ;
		LeftAnalogPrevY = &FDirectInputJoystick::RotPrevZ;
	break;
	}
}

void FDirectInputJoystick::SetDelegateRightAnalogX(EDirectInputPadKeyNames ePadKey)
{
	switch(ePadKey)
	{
	case DIGamePad_AXIS_X:
		RightAnalogX	 = &FDirectInputJoystick::X;
		RightAnalogPrevX = &FDirectInputJoystick::PrevX;
	break;

	case DIGamePad_AXIS_Y:
		RightAnalogX	 = &FDirectInputJoystick::Y;
		RightAnalogPrevX = &FDirectInputJoystick::PrevY;
	break;

	case DIGamePad_AXIS_Z:
		RightAnalogX	 = &FDirectInputJoystick::Z;
		RightAnalogPrevX = &FDirectInputJoystick::PrevZ;
	break;

	case DIGamePad_ROT_X:
		RightAnalogX	 = &FDirectInputJoystick::RotX;
		RightAnalogPrevX = &FDirectInputJoystick::RotPrevX;
	break;

	case DIGamePad_ROT_Y:
		RightAnalogX	 = &FDirectInputJoystick::RotY;
		RightAnalogPrevX = &FDirectInputJoystick::RotPrevY;
	break;

	case DIGamePad_ROT_Z:
		RightAnalogX	 = &FDirectInputJoystick::RotZ;
		RightAnalogPrevX = &FDirectInputJoystick::RotPrevZ;
	break;
	}
}

void FDirectInputJoystick::SetDelegateRightAnalogY(EDirectInputPadKeyNames ePadKey)
{
	switch(ePadKey)
	{
	case DIGamePad_AXIS_X:
		RightAnalogY	 = &FDirectInputJoystick::X;
		RightAnalogPrevY = &FDirectInputJoystick::PrevX;
	break;

	case DIGamePad_AXIS_Y:
		RightAnalogY	 = &FDirectInputJoystick::Y;
		RightAnalogPrevY = &FDirectInputJoystick::PrevY;
	break;

	case DIGamePad_AXIS_Z:
		RightAnalogY	 = &FDirectInputJoystick::Z;
		RightAnalogPrevY = &FDirectInputJoystick::PrevZ;
	break;

	case DIGamePad_ROT_X:
		RightAnalogY	 = &FDirectInputJoystick::RotX;
		RightAnalogPrevY = &FDirectInputJoystick::RotPrevX;
	break;

	case DIGamePad_ROT_Y:
		RightAnalogY	 = &FDirectInputJoystick::RotY;
		RightAnalogPrevY = &FDirectInputJoystick::RotPrevY;
	break;

	case DIGamePad_ROT_Z:
		RightAnalogY	 = &FDirectInputJoystick::RotZ;
		RightAnalogPrevY = &FDirectInputJoystick::RotPrevZ;
	break;
	}
}

/////////////////////////////
// 軸
/////////////////////////////
namespace{
// スレッショルドの中にあるかをチェック
inline bool is_thr_inner(int32_t n, uint32_t nThreshould)
{
	int32_t nThr = static_cast<int32_t>(nThreshould);
	return -nThr<=n && n<=nThr;
}

inline float calc_axis_ratio(int32_t n, uint32_t nThreshould, bool bReverse=false)
{
	float r = static_cast<float>(::abs(n)-nThreshould)/static_cast<float>(FDirectInputJoystick::MAX_AXIS_VALUE-nThreshould);

	if(!bReverse)
		return n<0 ? -r : r;
	else
		return n<0 ? r : -r;
}
} // namespace end

float FDirectInputJoystick::X()const
{
	int32_t nX = joyBuf_[nCurIndex_].lX;

	if(is_thr_inner(nX, nX_Threshold_)) return 0.f;
	return calc_axis_ratio(nX, nX_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_X]);
}

float FDirectInputJoystick::Y()const
{
	int32_t nY = joyBuf_[nCurIndex_].lY;

	if(is_thr_inner(nY, nY_Threshold_)) return 0.f;
	return calc_axis_ratio(nY, nY_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_Y]);
}

float FDirectInputJoystick::Z()const
{
	int32_t nZ = joyBuf_[nCurIndex_].lZ;
	
	if(is_thr_inner(nZ, nZ_Threshold_)) return 0.f;
	return calc_axis_ratio(nZ, nZ_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_Z]);
}

float FDirectInputJoystick::PrevX()const
{
	int32_t nX = joyBuf_[nCurIndex_^1].lX;

	if(is_thr_inner(nX, nX_Threshold_)) return 0.f;
	return calc_axis_ratio(nX, nX_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_X]);
}

float FDirectInputJoystick::PrevY()const
{
	int32_t nY = joyBuf_[nCurIndex_^1].lY;

	if(is_thr_inner(nY, nY_Threshold_)) return 0.f;
	return calc_axis_ratio(nY, nY_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_Y]);
}

float FDirectInputJoystick::PrevZ()const
{
	int32_t nZ = joyBuf_[nCurIndex_^1].lZ;
	
	if(is_thr_inner(nZ, nZ_Threshold_)) return 0.f;
	return calc_axis_ratio(nZ, nZ_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_Z]);
}

//////////////////////////////
// 回転
/////////////////////////////
float FDirectInputJoystick::RotX()const
{
	int32_t nXrot = joyBuf_[nCurIndex_].lRx;

	if(is_thr_inner(nXrot, nXrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nXrot, nXrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_X]);
}

float FDirectInputJoystick::RotY()const
{
	int32_t nYrot = joyBuf_[nCurIndex_].lRy;

	if(is_thr_inner(nYrot, nYrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nYrot, nYrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_Y]);
}

float FDirectInputJoystick::RotZ()const
{
	int32_t nZrot = joyBuf_[nCurIndex_].lRz;

	if(is_thr_inner(nZrot, nZrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nZrot, nZrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_Z]);
}

float FDirectInputJoystick::RotPrevX()const
{
	int32_t nXrot = joyBuf_[nCurIndex_^1].lRx;

	if(is_thr_inner(nXrot, nXrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nXrot, nXrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_X]);
}

float FDirectInputJoystick::RotPrevY()const
{
	int32_t nYrot = joyBuf_[nCurIndex_^1].lRy;

	if(is_thr_inner(nYrot, nYrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nYrot, nYrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_Y]);
}

float FDirectInputJoystick::RotPrevZ()const
{
	int32_t nZrot = joyBuf_[nCurIndex_^1].lRz;

	if(is_thr_inner(nZrot, nZrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nZrot, nZrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_Z]);
}

//////////////////////////////
// POV
/////////////////////////////
int32 FDirectInputJoystick::Pov()const
{
	return joyBuf_[nCurIndex_].rgdwPOV[0];
}

int32 FDirectInputJoystick::PrevPov()const
{
	return joyBuf_[nCurIndex_^1].rgdwPOV[0];
}

bool FDirectInputJoystick::IsPovPress(enum EDirectInputArrow eArrow)const
{
	if(IsAdConvFlag() 
	&& IsAxisPressInner(SwapPovAxis(eArrow), nCurIndex_))
		return true;

	return IsPovPressInner(eArrow, nCurIndex_);
}

bool FDirectInputJoystick::IsPovPush(enum EDirectInputArrow eArrow)const
{
	if( IsAdConvFlag()
	&&  IsAxisPressInner(SwapPovAxis(eArrow), nCurIndex_)
	&& !IsAxisPressInner(SwapPovAxis(eArrow), nCurIndex_^1))
		return true;

	return IsPovPressInner(eArrow, nCurIndex_)
		&& !IsPovPressInner(eArrow, nCurIndex_^1);
}

bool FDirectInputJoystick::IsPovRelease(enum EDirectInputArrow eArrow)const
{
	if( IsAdConvFlag()
	&& !IsAxisPressInner(SwapPovAxis(eArrow), nCurIndex_)
	&&  IsAxisPressInner(SwapPovAxis(eArrow), nCurIndex_^1))
		return true;

	return !IsPovPressInner(eArrow, nCurIndex_)
		&&  IsPovPressInner(eArrow, nCurIndex_^1);
}

bool FDirectInputJoystick::IsPovPressInner(enum EDirectInputArrow eArrow, uint32_t nIndex)const
{
	DWORD pov = joyBuf_[nIndex].rgdwPOV[0];

	// 斜め入力対応
	switch(eArrow)
	{
	case POV_UP:	return pov==0     || pov==4500  || pov==31500;
	case POV_RIGHT:	return pov==9000  || pov==4500  || pov==13500;
	case POV_DOWN:	return pov==18000 || pov==13500 || pov==22500;
	case POV_LEFT:	return pov==27000 || pov==22500 || pov==31500;
	case POV_NONE:	return LOWORD(pov)==0xFFFF;
	default:		return false;
	}
}

bool FDirectInputJoystick::IsAxisPress(enum EDirectInputArrow eArrow)const
{
	if(IsAdConvFlag() 
	&& IsPovPressInner(SwapPovAxis(eArrow), nCurIndex_))
		return true;

	return IsAxisPressInner(eArrow, nCurIndex_);
}

bool FDirectInputJoystick::IsAxisPush(enum EDirectInputArrow eArrow)const
{
	if( IsAdConvFlag()
	&&  IsPovPressInner(SwapPovAxis(eArrow), nCurIndex_)
	&& !IsPovPressInner(SwapPovAxis(eArrow), nCurIndex_^1))
		return true;

	return  IsAxisPressInner(eArrow, nCurIndex_)
		&& !IsAxisPressInner(eArrow, nCurIndex_^1);
}

bool FDirectInputJoystick::IsAxisRelease(enum EDirectInputArrow eArrow)const
{
	if( IsAdConvFlag()
	&& !IsPovPressInner(SwapPovAxis(eArrow), nCurIndex_)
	&&  IsPovPressInner(SwapPovAxis(eArrow), nCurIndex_^1))
		return true;

	return !IsAxisPressInner(eArrow, nCurIndex_)
		&&  IsAxisPressInner(eArrow, nCurIndex_^1);
}

bool FDirectInputJoystick::IsAxisPressInner(enum EDirectInputArrow eArrow, uint32_t nIndex)const
{
	bool bCur = (nIndex==nCurIndex_);
	switch (eArrow)
	{
	case AXIS_UP:	return (bCur ? LeftAnalogY(*this) : LeftAnalogPrevY(*this)) < -0.6f;
	case AXIS_RIGHT:return (bCur ? LeftAnalogX(*this) : LeftAnalogPrevX(*this)) >  0.6f;
	case AXIS_DOWN:	return (bCur ? LeftAnalogY(*this) : LeftAnalogPrevY(*this)) >  0.6f;
	case AXIS_LEFT:	return (bCur ? LeftAnalogX(*this) : LeftAnalogPrevX(*this)) < -0.6f;
	case AXIS_NONE:	return (bCur ? LeftAnalogX(*this) : LeftAnalogPrevX(*this))==0.f && (bCur ? LeftAnalogY(*this) : LeftAnalogPrevY(*this))==0.f;
	default:		return false;
	}
}


bool FDirectInputJoystick::IsAxisRightPress(enum EDirectInputArrow eArrow)const
{
	return IsAxisRightPressInner(eArrow, nCurIndex_);
}

bool FDirectInputJoystick::IsAxisRightPush(enum EDirectInputArrow eArrow)const
{
	return  IsAxisRightPressInner(eArrow, nCurIndex_)
		&& !IsAxisRightPressInner(eArrow, nCurIndex_^1);
}

bool FDirectInputJoystick::IsAxisRightRelease(enum EDirectInputArrow eArrow)const
{
	return !IsAxisRightPressInner(eArrow, nCurIndex_)
		&&  IsAxisRightPressInner(eArrow, nCurIndex_^1);
}

bool FDirectInputJoystick::IsAxisRightPressInner(enum EDirectInputArrow eArrow, uint32_t nIndex)const
{
	bool bCur = (nIndex==nCurIndex_);
	switch (eArrow)
	{
	case AXIS_UP:	return (bCur ? RightAnalogY(*this) : RightAnalogPrevY(*this)) < -0.6f;
	case AXIS_RIGHT:return (bCur ? RightAnalogX(*this) : RightAnalogPrevX(*this)) >  0.6f;
	case AXIS_DOWN:	return (bCur ? RightAnalogY(*this) : RightAnalogPrevY(*this)) >  0.6f;
	case AXIS_LEFT:	return (bCur ? RightAnalogX(*this) : RightAnalogPrevX(*this)) < -0.6f;
	case AXIS_NONE:	return (bCur ? RightAnalogX(*this) : RightAnalogPrevX(*this))==0.f && (bCur ? RightAnalogY(*this) : RightAnalogPrevY(*this))==0.f;
	default:		return false;
	}
}

enum EDirectInputArrow FDirectInputJoystick::SwapPovAxis(enum EDirectInputArrow eArrow)const
{
	switch(eArrow)
	{
	case POV_UP:	return AXIS_UP;
	case POV_RIGHT: return AXIS_RIGHT;
	case POV_DOWN:  return AXIS_DOWN;
	case POV_LEFT:  return AXIS_LEFT;
	case AXIS_UP:	return POV_UP;
	case AXIS_RIGHT:return POV_RIGHT;
	case AXIS_DOWN:	return POV_DOWN;
	case AXIS_LEFT:	return POV_LEFT;
	case POV_NONE:  return AXIS_NONE;
	case AXIS_NONE: return POV_NONE;
	default:		return ARROW_END;
	}
}

//////////////////////////////
// ボタン
/////////////////////////////
bool FDirectInputJoystick::IsPress(uint32_t nBtn)const
{
	if(nBtn>=ARROW_END)	return false;

	switch(nBtn)
	{
	case POV_UP:
	case POV_RIGHT:
	case POV_DOWN:
	case POV_LEFT:
	case POV_NONE:
		return IsPovPress(static_cast<enum EDirectInputArrow>(nBtn));
	break;

	case AXIS_UP:
	case AXIS_RIGHT:
	case AXIS_DOWN:
	case AXIS_LEFT:
	case AXIS_NONE:
		return IsAxisPress(static_cast<enum EDirectInputArrow>(nBtn));
	break;
	}

	return (joyBuf_[nCurIndex_].rgbButtons[nBtn] & 0x80)>0;
}

bool FDirectInputJoystick::IsPush(uint32_t nBtn)const
{
	if(nBtn >= ARROW_END)	return false;

	switch(nBtn)
	{
	case POV_UP:
	case POV_RIGHT:
	case POV_DOWN:
	case POV_LEFT:
	case POV_NONE:
	return IsPovPush(static_cast<enum EDirectInputArrow>(nBtn));
	break;

	case AXIS_UP:
	case AXIS_RIGHT:
	case AXIS_DOWN:
	case AXIS_LEFT:
	case AXIS_NONE:
	return IsAxisPush(static_cast<enum EDirectInputArrow>(nBtn));
	break;
	}

	return (joyBuf_[nCurIndex_].rgbButtons[nBtn] & 0x80)>0
		&& (joyBuf_[nCurIndex_ ^ 1].rgbButtons[nBtn] & 0x80) == 0;
}

bool FDirectInputJoystick::IsRelease(uint32_t nBtn)const
{
	if(nBtn>=ARROW_END)	return false;

	switch(nBtn)
	{
	case POV_UP:
	case POV_RIGHT:
	case POV_DOWN:
	case POV_LEFT:
	case POV_NONE:
		return IsPovRelease(static_cast<enum EDirectInputArrow>(nBtn));
	break;

	case AXIS_UP:
	case AXIS_RIGHT:
	case AXIS_DOWN:
	case AXIS_LEFT:
	case AXIS_NONE:
		return IsAxisRelease(static_cast<enum EDirectInputArrow>(nBtn));
	break;
	}

	return (joyBuf_[nCurIndex_].rgbButtons[nBtn] & 0x80)==0
		&& (joyBuf_[nCurIndex_^1].rgbButtons[nBtn] & 0x80)>0;
}

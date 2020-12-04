#include "DirectInputJoystick.h"
#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputPadState.h"
#include "DirectInputDriver.h"

#include "Windows/AllowWindowsPlatformTypes.h"

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
		UE_LOG(LogDirectInputPadPlugin, Error, TEXT("Joystick CreateDevice fail. : %x"),r);
		return false;
	}

	r = pDevice_->SetDataFormat(&c_dfDIJoystick);
	if(r!=DI_OK)
	{
		UE_LOG(LogDirectInputPadPlugin, Error, TEXT("Joystick SetDataformat fail. : %x"),r);
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
		UE_LOG(LogDirectInputPadPlugin, Error, TEXT("Joystick SetCooperativeLevel fail. : %x"), r);
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
		UE_LOG(LogDirectInputPadPlugin, Error, TEXT("Joystick AxisMode Setup fail. : %x"), r);
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

	// 軸の反転フラグ設定
	//AxisReverseFlagMap_.SetNumUninitialized(DIGamePad_ROT_Z+1);
	//for(uint32 i=DIGamePad_AXIS_X; i<=DIGamePad_ROT_Z; ++i)
	//	AxisReverseFlagMap_[i] = false;

	AxisReverseFlagMap_.Init(false,DIGamePad_ROT_Z+1);
	AxisReverseFlagMap_[DIGamePad_AXIS_Y] = true; // Y軸はXInputと合わせるためにデフォルトで反転

	// 初期化
	memset(&InitialJoyBuf_,0,sizeof(DIJOYSTATE));
	memset(&joyBuf_[0],0,sizeof(DIJOYSTATE));
	memset(&joyBuf_[1],0,sizeof(DIJOYSTATE));

	// 初期値を得る
	int loopcount=0;
	do{
		FPlatformProcess::Sleep(0.2);
		if(Input())
		{
			InitialJoyBuf_ = joyBuf_[nCurIndex_];
			if(++loopcount>10) break;
		}
		if(loopcount>100)
		{
			UE_LOG(LogDirectInputPadPlugin, Log, TEXT("DirectInput Joystick Create Fail. : %s"), joyins.tszProductName);
			return false;
		}
	}while(true);

	//	const string sFlag = (flags&DISCL_BACKGROUND)>0 ? "BACKGROUND" : "FOREGROUND";
	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("DirectInput Joystick Create Success. : %s"), joyins.tszProductName);
	//UE_LOG(LogDirectInputPadPlugin, Log, TEXT("Init: %d"), InitialJoyBuf_.lRx);

	return true;
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
}

void FDirectInputJoystick::EventButtonReleased(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, EDirectInputPadKeyNames ePadName, FKey DIKey)
{
	if(!IsRelease(ePadName-DIGamePad_Button1)) return;

	MessageHandler->OnControllerButtonReleased(DIKey.GetFName(), GetPlayerIndex(), false);
}

void FDirectInputJoystick::EventPov(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
	if(IsPush(POV_UP))
	{
		MessageHandler->OnControllerButtonPressed(EKeysDirectInputPad::DIGamePad_POV_Up.GetFName(), GetPlayerIndex(), false);
	}
	else if(IsRelease(POV_UP))
	{
		MessageHandler->OnControllerButtonReleased(EKeysDirectInputPad::DIGamePad_POV_Up.GetFName(), GetPlayerIndex(), false);
	}
	else if(IsPush(POV_DOWN))
	{
		MessageHandler->OnControllerButtonPressed(EKeysDirectInputPad::DIGamePad_POV_Down.GetFName(), GetPlayerIndex(), false);
	}
	else if(IsRelease(POV_DOWN))
	{
		MessageHandler->OnControllerButtonReleased(EKeysDirectInputPad::DIGamePad_POV_Down.GetFName(), GetPlayerIndex(), false);
	}

	if(IsPush(POV_RIGHT))
	{
		MessageHandler->OnControllerButtonPressed(EKeysDirectInputPad::DIGamePad_POV_Right.GetFName(), GetPlayerIndex(), false);
	}
	else if(IsRelease(POV_RIGHT))
	{
		MessageHandler->OnControllerButtonReleased(EKeysDirectInputPad::DIGamePad_POV_Right.GetFName(), GetPlayerIndex(), false);
	}
	else if(IsPush(POV_LEFT))
	{
		MessageHandler->OnControllerButtonPressed(EKeysDirectInputPad::DIGamePad_POV_Left.GetFName(), GetPlayerIndex(), false);
	}
	else if(IsRelease(POV_LEFT))
	{
		MessageHandler->OnControllerButtonReleased(EKeysDirectInputPad::DIGamePad_POV_Left.GetFName(), GetPlayerIndex(), false);
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

bool FDirectInputJoystick::IsChangedKeyState()const
{
//	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("x %d %d"), InitialJoyBuf_.lX, joyBuf_[nCurIndex_].lX);
//	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("y %d %d"), joyBuf_[nCurIndex_].lY, InitialJoyBuf_.lY);
//	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("z %d %d"), joyBuf_[nCurIndex_].lZ, InitialJoyBuf_.lZ);

//	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("Rx %d %d"), joyBuf_[nCurIndex_].lRx, InitialJoyBuf_.lRx);
//	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("Ry %d %d"), joyBuf_[nCurIndex_].lRy, InitialJoyBuf_.lRy);
//	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("Rz %d %d"), joyBuf_[nCurIndex_].lRz, InitialJoyBuf_.lRz);

	// 軸からチェック。初期値と比較
	if(InitX()!=X()
	|| InitY()!=Y()
	|| InitZ()!=Z()
	|| InitRotX()!=RotX()
	|| InitRotY()!=RotY()
	|| InitRotZ()!=RotZ())
		return true;

	// ボタンは、PushもReleaseもされてない
	for(uint32 i = 0; i<32; ++i)
	{
		if(((joyBuf_[nCurIndex_].rgbButtons[i]&0x80)>0)||((joyBuf_[1^nCurIndex_].rgbButtons[i]&0x80)>0))
			return true;
	}

	// POVも押されていない
	if(LOWORD(joyBuf_[nCurIndex_].rgdwPOV[0])!=0xFFFF) return true;

	//UE_LOG(LogDirectInputPadPlugin, Log, TEXT("IsChangedState: false"));
	return false;
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
//////////////////////////////
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
// 初期軸
//////////////////////////////
float FDirectInputJoystick::InitX()const
{
	int32_t nX = InitialJoyBuf_.lX;

	if(is_thr_inner(nX, nX_Threshold_)) return 0.f;
	return calc_axis_ratio(nX, nX_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_X]);
}

float FDirectInputJoystick::InitY()const
{
	int32_t nY = InitialJoyBuf_.lY;

	if(is_thr_inner(nY, nY_Threshold_)) return 0.f;
	return calc_axis_ratio(nY, nY_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_Y]);
}

float FDirectInputJoystick::InitZ()const
{
	int32_t nZ = InitialJoyBuf_.lZ;

	if(is_thr_inner(nZ, nZ_Threshold_)) return 0.f;
	return calc_axis_ratio(nZ, nZ_Threshold_, AxisReverseFlagMap_[DIGamePad_AXIS_Z]);
}

float FDirectInputJoystick::InitRotX()const
{
	int32_t nRotX = InitialJoyBuf_.lRx;

	if(is_thr_inner(nRotX, nXrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nRotX, nXrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_X]);
}

float FDirectInputJoystick::InitRotY()const
{
	int32_t nRotY = InitialJoyBuf_.lRy;

	if(is_thr_inner(nRotY, nYrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nRotY, nYrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_Y]);
}

float FDirectInputJoystick::InitRotZ()const
{
	int32_t nRotZ = InitialJoyBuf_.lRz;

	if(is_thr_inner(nRotZ, nZrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nRotZ, nZrot_Threshold_, AxisReverseFlagMap_[DIGamePad_ROT_Z]);
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
	case AXIS_UP:	return (bCur ? Y() : PrevY()) >  0.6f;
	case AXIS_RIGHT:return (bCur ? X() : PrevX()) >  0.6f;
	case AXIS_DOWN:	return (bCur ? Y() : PrevY()) < -0.6f;
	case AXIS_LEFT:	return (bCur ? X() : PrevX()) < -0.6f;
	case AXIS_NONE:	return (bCur ? X() : PrevX())==0.f && (bCur ? Y() : PrevY())==0.f;
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
	case AXIS_UP:	return (bCur ? RotZ()	: RotPrevZ())	>  0.6f;
	case AXIS_RIGHT:return (bCur ? Z()		: PrevZ())		>  0.6f;
	case AXIS_DOWN:	return (bCur ? RotZ()	: RotPrevZ())	< -0.6f;
	case AXIS_LEFT:	return (bCur ? Z()		: PrevZ())		< -0.6f;
	case AXIS_NONE:	return (bCur ? Z() : PrevZ())==0.f && (bCur ? RotZ() : RotPrevZ())==0.f;
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

bool FDirectInputJoystick::IsPrevPress(uint32_t nBtn)const
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

	return (joyBuf_[nCurIndex_^1].rgbButtons[nBtn] & 0x80)>0;
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

#include "Windows/HideWindowsPlatformTypes.h"

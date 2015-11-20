#include "DirectInputPadPluginPrivatePCH.h"
#include "DirectInputJoystick.h"

#include <tuple>

DEFINE_LOG_CATEGORY_STATIC(DirectInputPadPlugin, Log, All)

//////////////////////////////////////
// FDirectInputDrive
/////////////////////////////////////
bool FDirectInputDriver::Init()
{
	if(pDriver_) return true;

	HRESULT r = DirectInput8Create(::GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8,
		reinterpret_cast<LPVOID*>(&pDriver_), NULL);

	if(r != DI_OK)
	{
		UE_LOG(DirectInputPlugin, Error, TEXT("DirectInputDriver initialize fail."));
		return false;
	}

	UE_LOG(DirectInputPlugin, Log, TEXT("DirectInputDriver Initialied."));
	return true;
}

void FDirectInputDriver::Fin()
{
	pDriver_->Release();
	pDriver_ = nullptr;
}

//////////////////////////////////////
// FDirectInputJoystickEnum
/////////////////////////////////////
bool FDirectInputJoystickEnum::Init(FDirectInputDriver& adapter)
{
	vecJoyStickInfo_.Reset();
	auto pDriver = adapter.driver();

	HRESULT r = pDriver->EnumDevices(DI8DEVCLASS_GAMECTRL, &FDirectInputJoystickEnum::OnEnumDevice, this, DIEDFL_ATTACHEDONLY);
	if(FAILED(r))
	{
		UE_LOG(DirectInputPlugin, Warning, TEXT("Search Joysticks Error."));
		return false;
	}

	vecJoyStickInfo_.Shrink();
	return true;
}

const DIDEVICEINSTANCE* FDirectInputJoystickEnum::GetJoystickInfo(uint32 nJoyNo)const
{
	if(!vecJoyStickInfo_.IsValidIndex(nJoyNo))
	{
		UE_LOG(DirectInputPlugin, Warning, TEXT("Unsuable Joystick ： No. %n"), nJoyNo);
		return nullptr;
	}

	return &(vecJoyStickInfo_[nJoyNo]);
}

BOOL CALLBACK FDirectInputJoystickEnum::OnEnumDevice(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	FDirectInputJoystickEnum* pEnum = reinterpret_cast<FDirectInputJoystickEnum*>(pvRef);

	DIDEVICEINSTANCE di = *lpddi;
	pEnum->vecJoyStickInfo_.Emplace(di);

	//logger::traceln(di.tszProductName);

	return DIENUM_CONTINUE;
}

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
		UE_LOG(DirectInputPlugin, Error, TEXT("Joystick CreateDevice fail."));
		return false;
	}

	r = pDevice_->SetDataFormat(&c_dfDIJoystick);
	if(r!=DI_OK)
	{
		UE_LOG(DirectInputPlugin, Error, TEXT("Joystick SetDataformat fail."));
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
		UE_LOG(DirectInputPlugin, Error, TEXT("Joystick SetCooperativeLevel fail."));
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
		UE_LOG(DirectInputPlugin, Error, TEXT("Joystick AxisMode Setup fail."));
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

//	const string sFlag = (flags&DISCL_BACKGROUND)>0 ? "BACKGROUND" : "FOREGROUND";
	UE_LOG(DirectInputPlugin, Log, TEXT("Joystick Device Create Success."));

	return true;
}

void FDirectInputJoystick::Fin()
{
	if(pDevice_ && bAcquire_) pDevice_->Unacquire();
	pDevice_->Release();
	pDevice_=nullptr;
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

void FDirectInputJoystick::SetGuard(bool bGuard)
{
	bGuard_ = bGuard;
	if(bGuard_) ClearBuf();
}

//////////////////////////////
// 軸
/////////////////////////////
namespace{
// スレッショルドの中にあるかをチェック
inline bool is_thr_inner(int32_t n, uint32_t nThreshould)
{
	int32_t nThr = static_cast<int32_t>(nThreshould);
	return -nThr<=n && n<=nThr;
}

inline float calc_axis_ratio(int32_t n, uint32_t nThreshould)
{
	float r = static_cast<float>(::abs(n)-nThreshould)/static_cast<float>(FDirectInputJoystick::MAX_AXIS_VALUE-nThreshould);
	return n<0 ? -r : r;
}
} // namespace end

float FDirectInputJoystick::X()const
{
	int32_t nX = joyBuf_[nCurIndex_].lX;

	if(is_thr_inner(nX, nX_Threshold_)) return 0.f;
	return calc_axis_ratio(nX, nX_Threshold_);
}

float FDirectInputJoystick::Y()const
{
	int32_t nY = joyBuf_[nCurIndex_].lY;

	if(is_thr_inner(nY, nY_Threshold_)) return 0.f;
	return calc_axis_ratio(nY, nY_Threshold_);
}

float FDirectInputJoystick::Z()const
{
	int32_t nZ = joyBuf_[nCurIndex_].lZ;
	
	if(is_thr_inner(nZ, nZ_Threshold_)) return 0.f;
	return calc_axis_ratio(nZ, nZ_Threshold_);
}

float FDirectInputJoystick::PrevX()const
{
	int32_t nX = joyBuf_[nCurIndex_^1].lX;

	if(is_thr_inner(nX, nX_Threshold_)) return 0.f;
	return calc_axis_ratio(nX, nX_Threshold_);
}

float FDirectInputJoystick::PrevY()const
{
	int32_t nY = joyBuf_[nCurIndex_^1].lY;

	if(is_thr_inner(nY, nY_Threshold_)) return 0.f;
	return calc_axis_ratio(nY, nY_Threshold_);
}

float FDirectInputJoystick::PrevZ()const
{
	int32_t nZ = joyBuf_[nCurIndex_^1].lZ;
	
	if(is_thr_inner(nZ, nZ_Threshold_)) return 0.f;
	return calc_axis_ratio(nZ, nZ_Threshold_);
}

//////////////////////////////
// 回転
/////////////////////////////
float FDirectInputJoystick::RotX()const
{
	int32_t nXrot = joyBuf_[nCurIndex_].lRx;

	if(is_thr_inner(nXrot, nXrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nXrot, nXrot_Threshold_);
}

float FDirectInputJoystick::RotY()const
{
	int32_t nYrot = joyBuf_[nCurIndex_].lRy;

	if(is_thr_inner(nYrot, nYrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nYrot, nYrot_Threshold_);
}

float FDirectInputJoystick::RotZ()const
{
	int32_t nZrot = joyBuf_[nCurIndex_].lRz;

	if(is_thr_inner(nZrot, nZrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nZrot, nZrot_Threshold_);
}

float FDirectInputJoystick::RotPrevX()const
{
	int32_t nXrot = joyBuf_[nCurIndex_^1].lRx;

	if(is_thr_inner(nXrot, nXrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nXrot, nXrot_Threshold_);
}

float FDirectInputJoystick::RotPrevY()const
{
	int32_t nYrot = joyBuf_[nCurIndex_^1].lRy;

	if(is_thr_inner(nYrot, nYrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nYrot, nYrot_Threshold_);
}

float FDirectInputJoystick::RotPrevZ()const
{
	int32_t nZrot = joyBuf_[nCurIndex_^1].lRz;

	if(is_thr_inner(nZrot, nZrot_Threshold_)) return 0.f;
	return calc_axis_ratio(nZrot, nZrot_Threshold_);
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
	case POV_NONE:	return LOWORD(pov)==0xFFFF;
	case POV_UP:	return pov==0     || pov==4500  || pov==31500;
	case POV_RIGHT:	return pov==9000  || pov==4500  || pov==13500;
	case POV_DOWN:	return pov==18000 || pov==13500 || pov==22500;
	case POV_LEFT:	return pov==27000 || pov==22500 || pov==31500;
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
	case AXIS_NONE:	return (bCur ? X() : PrevX())==0.f && (bCur ? Y() : PrevY())==0.f;
	case AXIS_UP:	return (bCur ? Y() : PrevY())<0.f;
	case AXIS_RIGHT:return (bCur ? X() : PrevX())>0.f;
	case AXIS_DOWN:	return (bCur ? Y() : PrevY())>0.f;
	case AXIS_LEFT:	return (bCur ? X() : PrevX())<0.f;
	default:		return false;
	}
}

enum EDirectInputArrow FDirectInputJoystick::SwapPovAxis(enum EDirectInputArrow eArrow)const
{
	switch(eArrow)
	{
	case AXIS_NONE: return POV_NONE;
	case AXIS_UP:	return POV_UP;
	case AXIS_RIGHT:return POV_RIGHT;
	case AXIS_DOWN:	return POV_DOWN;
	case AXIS_LEFT:	return POV_LEFT;
	case POV_NONE:  return AXIS_NONE;
	case POV_UP:	return AXIS_UP;
	case POV_RIGHT: return AXIS_RIGHT;
	case POV_DOWN:  return AXIS_DOWN;
	case POV_LEFT:  return AXIS_LEFT;
	default:		return ARROW_END;
	}
}

//////////////////////////////
// ボタン
/////////////////////////////
bool FDirectInputJoystick::IsPress(uint32_t nBtn)const
{
	if(nBtn>=ARROW_END)	return false;
	if(nBtn>=AXIS_NONE) return IsAxisPress(static_cast<enum EDirectInputArrow>(nBtn));
	if(nBtn>=POV_NONE)	return IsPovPress(static_cast<enum EDirectInputArrow>(nBtn));

	return (joyBuf_[nCurIndex_].rgbButtons[nBtn] & 0x80)>0;
}

bool FDirectInputJoystick::IsPush(uint32_t nBtn)const
{
	if(nBtn>=ARROW_END)	return false; 
	if(nBtn>=AXIS_NONE) return IsAxisPush(static_cast<enum EDirectInputArrow>(nBtn));
	if(nBtn>=POV_NONE)	return IsPovPush(static_cast<enum EDirectInputArrow>(nBtn));

	return (joyBuf_[nCurIndex_].rgbButtons[nBtn] & 0x80)>0
		&& (joyBuf_[nCurIndex_^1].rgbButtons[nBtn] & 0x80)==0;
}

bool FDirectInputJoystick::IsRelease(uint32_t nBtn)const
{
	if(nBtn>=ARROW_END)	return false;
	if(nBtn>=AXIS_NONE) return IsAxisRelease(static_cast<enum EDirectInputArrow>(nBtn));
	if(nBtn>=POV_NONE)	return IsPovRelease(static_cast<enum EDirectInputArrow>(nBtn));

	return (joyBuf_[nCurIndex_].rgbButtons[nBtn] & 0x80)==0
		&& (joyBuf_[nCurIndex_^1].rgbButtons[nBtn] & 0x80)>0;
}

//////////////////////////////////////
// joystick_factory
/////////////////////////////////////
bool FDirectInputJoystickFactory::Init(HWND hWnd, const TSharedPtr<FDirectInputDriver>& pDriver, bool bBackGround)
{
	hWnd_		= hWnd;
	pAdapter_	= pDriver;
	bBackGround_= bBackGround;

	bool r = joyEnum_.Init(*pAdapter_);
	if(r) mapJoy_.Reserve(joyEnum_.EnabledJoystickNum());
	return r;
}

void FDirectInputJoystickFactory::Fin()
{
	mapJoy_.Reset();
}

TSharedPtr<FDirectInputJoystick> FDirectInputJoystickFactory::GetJoystick(uint32 nNo)
{
	auto joy = mapJoy_.Find(nNo);
	if(joy) return *joy;

	const DIDEVICEINSTANCE* info = joyEnum_.GetJoystickInfo(nNo);
	if(!info) return nullptr;
	
	TSharedPtr<FDirectInputJoystick> pJoy = MakeShareable<FDirectInputJoystick>(new FDirectInputJoystick());
	if(!pJoy->Init(*info, *pAdapter_, hWnd_, bBackGround_))
		return nullptr;
	
	return mapJoy_.Emplace(nNo, pJoy);
}

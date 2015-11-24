#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputPadState.h"
#include "DirectInputJoystick.h"

#include "DirectInputDriver.h"

#include <tuple>

DEFINE_LOG_CATEGORY_STATIC(DirectInputPadPlugin, Log, All)

#include "AllowWindowsPlatformTypes.h"

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
		UE_LOG(DirectInputPadPlugin, Error, TEXT("DirectInputDriver initialization failed."));
		return false;
	}

	UE_LOG(DirectInputPadPlugin, Log, TEXT("DirectInputDriver initialized."));
	return true;
}

void FDirectInputDriver::Fin()
{
	if(pDriver_)
	{
		pDriver_->Release();
		pDriver_ = nullptr;
	}
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
		UE_LOG(DirectInputPadPlugin, Warning, TEXT("Search DirectInputPad Error."));
		return false;
	}

	vecJoyStickInfo_.Shrink();
	return true;
}

const DIDEVICEINSTANCE* FDirectInputJoystickEnum::GetJoystickInfo(uint32 nJoyNo)const
{
	if(!vecJoyStickInfo_.IsValidIndex(nJoyNo))
	{
		UE_LOG(DirectInputPadPlugin, Warning, TEXT("Unsuable DirectInputPadÅFNo. %n"), nJoyNo);
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
// joystick_factory
/////////////////////////////////////
FDirectInputJoystickFactory::FDirectInputJoystickFactory(){}

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
	if(joy && (*joy).IsValid()) return *joy;

	const DIDEVICEINSTANCE* info = joyEnum_.GetJoystickInfo(nNo);
	if(!info) return nullptr;
	
	TSharedPtr<FDirectInputJoystick> pJoy = MakeShareable<FDirectInputJoystick>(new FDirectInputJoystick());
	if(!pJoy->Init(*info, *pAdapter_, hWnd_, bBackGround_))
		return nullptr;
	
	return mapJoy_.Emplace(nNo, pJoy);
}

#include "HideWindowsPlatformTypes.h"

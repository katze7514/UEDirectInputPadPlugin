#include "DirectInputDriver.h"
#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputPadState.h"
#include "DirectInputJoystick.h"

#include <tuple>

#include "Windows/AllowWindowsPlatformTypes.h"

#include <wbemidl.h>
#include <oleauto.h>

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
		UE_LOG(LogDirectInputPadPlugin, Error, TEXT("DirectInputDriver initialization failed."));
		return false;
	}

	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("DirectInputDriver initialized."));
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
namespace{
// MSDN(https://msdn.microsoft.com/ja-jp/library/bb173051(v=vs.85).aspx)からの
// コピペと少し改変。DInputの情報からXInputかどうかを判定する関数らしい
// XInputと判定されたら,DInput情報配列から削除する
//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains 
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
// Unfortunately this information can not be found by just using DirectInput 
//-----------------------------------------------------------------------------
uint32 RemoveXInputDevice( TArray<DIDEVICEINSTANCE>& DeviceInstanceArray )
{
    IWbemLocator*           pIWbemLocator  = NULL;
    IEnumWbemClassObject*   pEnumDevices   = NULL;
    IWbemClassObject*       pDevices[20]   = {0};
    IWbemServices*          pIWbemServices = NULL;
    BSTR                    bstrNamespace  = NULL;
    BSTR                    bstrDeviceID   = NULL;
    BSTR                    bstrClassName  = NULL;
    DWORD                   uReturned      = 0;
    UINT                    iDevice        = 0;
    VARIANT                 var;
    HRESULT                 hr;

	uint32					nXInputDeviceNum = 0;

    // CoInit if needed
    hr = CoInitialize(NULL);
    bool bCleanupCOM = SUCCEEDED(hr);

    // Create WMI
    hr = CoCreateInstance( __uuidof(WbemLocator),
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(IWbemLocator),
                           (LPVOID*) &pIWbemLocator);
    if( FAILED(hr) || pIWbemLocator == NULL )
        goto LCleanup;

    bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" );if( bstrNamespace == NULL ) goto LCleanup;        
    bstrClassName = SysAllocString( L"Win32_PNPEntity" );   if( bstrClassName == NULL ) goto LCleanup;        
    bstrDeviceID  = SysAllocString( L"DeviceID" );          if( bstrDeviceID == NULL )  goto LCleanup;        
    
    // Connect to WMI 
    hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L, 
                                       0L, NULL, NULL, &pIWbemServices );
    if( FAILED(hr) || pIWbemServices == NULL )
        goto LCleanup;

    // Switch security level to IMPERSONATE. 
    CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                       RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );                    

    hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices ); 
    if( FAILED(hr) || pEnumDevices == NULL )
        goto LCleanup;

    // Loop over all devices
    for( ;; )
    {
        // Get 20 at a time
        hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
        if( FAILED(hr) )
            goto LCleanup;
        if( uReturned == 0 )
            break;

        for( iDevice=0; iDevice<uReturned; iDevice++ )
        {
            // For each device, get its device ID
            hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
            if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
            {
                // Check if the device ID contains "IG_".  If it does, then it's an XInput device
				    // This information can not be found from DirectInput 
                if( wcsstr( var.bstrVal, L"IG_" ) )
                {
                    // If it does, then get the VID/PID from var.bstrVal
                    uint32 dwPid = 0, dwVid = 0;
                    WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
                    if( strVid && swscanf_s( strVid, L"VID_%4X", &dwVid ) != 1 )
                        dwVid = 0;
                    WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                    if( strPid && swscanf_s( strPid, L"PID_%4X", &dwPid ) != 1 )
                        dwPid = 0;

                    // Compare the VID/PID to the DInput device
                    DWORD dwVidPid = MAKELONG( dwVid, dwPid );

					// 取得済みのDirectInputデバイス情報からXInputのものを探す
					// XInputだったら配列から削除しておく
					for(int32 i=0; i<DeviceInstanceArray.Num(); ++i)
					{
						auto& DDevice = DeviceInstanceArray[i];
						if( dwVidPid == DDevice.guidProduct.Data1 )
						{
							DeviceInstanceArray.RemoveAt(i,1,false);
							++nXInputDeviceNum;
							break;
						}
					}
                }
            }   
            SAFE_RELEASE( pDevices[iDevice] );
        }
    }

LCleanup:
    if(bstrNamespace)
        SysFreeString(bstrNamespace);
    if(bstrDeviceID)
        SysFreeString(bstrDeviceID);
    if(bstrClassName)
        SysFreeString(bstrClassName);
    for( iDevice=0; iDevice<20; iDevice++ )
        SAFE_RELEASE( pDevices[iDevice] );
    SAFE_RELEASE( pEnumDevices );
    SAFE_RELEASE( pIWbemLocator );
    SAFE_RELEASE( pIWbemServices );

    if( bCleanupCOM )
        CoUninitialize();

    return nXInputDeviceNum;
}
}

bool FDirectInputJoystickEnum::Init(FDirectInputDriver& adapter)
{
	vecJoyStickInfo_.Reset();
	auto pDriver = adapter.driver();

	HRESULT r = pDriver->EnumDevices(DI8DEVCLASS_GAMECTRL, &FDirectInputJoystickEnum::OnEnumDevice, this, DIEDFL_ATTACHEDONLY);
	if(FAILED(r))
	{
		UE_LOG(LogDirectInputPadPlugin, Warning, TEXT("Search DirectInputPad Error."));
		return false;
	}

	nXInputDeviceNum_ = RemoveXInputDevice(vecJoyStickInfo_);

	vecJoyStickInfo_.Shrink();
	return true;
}

const DIDEVICEINSTANCE* FDirectInputJoystickEnum::GetJoystickInfo(uint32 nJoyNo)const
{
	if(!vecJoyStickInfo_.IsValidIndex(nJoyNo))
	{
		UE_LOG(LogDirectInputPadPlugin, Warning, TEXT("Unsuable DirectInputPad：No. %n"), nJoyNo);
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

#if WITH_EDITOR
	if(GIsEditor)
	{	bBackGround_ = true; }
	else
#endif
	{	bBackGround_= bBackGround; }

	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("Background %d"),bBackGround_);

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

#include "Windows/HideWindowsPlatformTypes.h"

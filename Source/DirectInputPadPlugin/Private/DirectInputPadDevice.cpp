#include "DirectInputPadDevice.h"
#include "DirectInputPadPluginPrivatePCH.h"

#if WITH_EDITOR
#include "MainFrame.h"
#endif

#include "DirectInputPadJoystick.h"

#include "DirectInputDriver.h"
#include "DirectInputJoystick.h"
#include "XInputJoystickEmu.h"

#include "Windows/AllowWindowsPlatformTypes.h"

namespace{
const uint32 MAX_JOYSTICKS = 8;
}

bool FDirectInputPadDevice::Init(bool bBackGround)
{
	UDirectInputPadFunctionLibrary::InitDirectInputPadJoystickLibrary();

	// MainWindowのウインドウハンドルを取得する
	HWND hWnd = NULL;
	TSharedPtr<SWindow> MainWindow;

#if WITH_EDITOR
	if(GIsEditor)
	{
		auto& MainFrameModule = IMainFrameModule::Get();
		MainWindow = MainFrameModule.GetParentWindow();
	}
	else
#endif
	{
		MainWindow = GEngine->GameViewport->GetWindow();
	}

	if(MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid())
	{
		hWnd = static_cast<HWND>(MainWindow->GetNativeWindow()->GetOSWindowHandle());
	}

	DDriver_ = MakeShareable<FDirectInputDriver>(new FDirectInputDriver());
	if(!DDriver_->Init())
	{
		bInit_ = false;
		return bInit_;
	}

	DFactory_ = MakeShareable<FDirectInputJoystickFactory>(new FDirectInputJoystickFactory());
	if(!DFactory_->Init(hWnd, DDriver_, bBackGround))
	{
		bInit_ = false;
		return bInit_;
	}

	XInputDeviceNum_ = DFactory_->GetXInputDeviceNum();
	DInputDeviceNum_ = 0;
	XJoysticks_.SetNum(MAX_JOYSTICKS);
	
	uint32 DJoyNum = MAX_JOYSTICKS - XInputDeviceNum_;
	DJoyNum =( DFactory_->EnabledJoystickNum() < DJoyNum) ? DFactory_->EnabledJoystickNum() : DJoyNum;

	UE_LOG(LogDirectInputPadPlugin, Log, TEXT("DirectInputPad detected: %d"), DJoyNum);

	for(uint32 i=0; i<DJoyNum; ++i)
	{
		//UE_LOG(LogDirectInputPadPlugin, Log, TEXT("DIPad %d"),i);

		auto joy = DFactory_->GetJoystick(i);
		if(joy.IsValid())
		{
			joy->SetPlayerIndex(XInputDeviceNum_+i);
			XJoysticks_[XInputDeviceNum_+i] = MakeShareable<FXInputJoystickEmu>(new FXInputJoystickEmu());
			XJoysticks_[XInputDeviceNum_+i]->Init(joy.ToSharedRef());

			++DInputDeviceNum_;
		}
	}

	bInit_ = true;
	return bInit_;
}

void FDirectInputPadDevice::Fin()
{
	UDirectInputPadFunctionLibrary::FinDirectInputPadJoystickLibrary();
	DFactory_->Fin();
	DDriver_->Fin();
}

void FDirectInputPadDevice::SendControllerEvents()
{
	if(DInputDeviceNum_==0) return;

	for(const auto& j : XJoysticks_)
	{
		if(j.IsValid())
		{
			// 入力に合わせてイベントを飛ばす
			j->Event(MessageHandler_);
		}
	}
}

TWeakPtr<FXInputJoystickEmu> FDirectInputPadDevice::GetJoystick(uint32 nPlayerID)
{
	if(!XJoysticks_.IsValidIndex(nPlayerID))
		return nullptr;

	if(XJoysticks_[nPlayerID].IsValid()) 
	return TWeakPtr<FXInputJoystickEmu>(XJoysticks_[nPlayerID]);

	return nullptr;
}

#include "Windows/HideWindowsPlatformTypes.h"

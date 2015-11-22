#include "DirectInputPadPluginPrivatePCH.h"

#include "MainFrame.h"

#include "DirectInputJoystick.h"
#include "DirectInputPadDevice.h"

DEFINE_LOG_CATEGORY_STATIC(DirectInputPadPlugin, Log, All)

namespace{
const uint32 MAX_JOYSTICKS = 8;
}

#include "AllowWindowsPlatformTypes.h"

bool FDirectInputPadDevice::Init(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	MessageHandler_ = InMessageHandler;

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
#else
	{
		MainWindow = GEngine->GameViewport->GetWindow();
	}
#endif

	if(MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid())
	{
		hWnd = static_cast<HWND>(MainWindow->GetNativeWindow()->GetOSWindowHandle());
	}

	DDriver_ = MakeShareable<FDirectInputDriver>(new FDirectInputDriver());
	if(!DDriver_->Init()) return false;

	DFactory_ = MakeShareable<FDirectInputJoystickFactory>(new FDirectInputJoystickFactory());
	if(!DFactory_->Init(hWnd, DDriver_)) return false;

	DJoysticks_.Reset(MAX_JOYSTICKS);
	
	uint32 DJoyNum = MAX_JOYSTICKS - XInputDeviceNum_;
	for(uint32 i= XInputDeviceNum_; i<DJoyNum; ++i)
	{
		auto joy = DFactory_->GetJoystick(i);
		if(joy.IsValid())
		{
			joy->SetPlayerID(i);
			DJoysticks_[i]=joy;
		}
	}

	return true;
}

void FDirectInputPadDevice::Fin()
{
	DFactory_->Fin();
	DDriver_->Fin();
}


void FDirectInputPadDevice::SendControllerEvents()
{
	if(DJoysticks_.Num()<=0) return;

	for(const auto& j : DJoysticks_)
	{
		if(j.IsValid())
		{
			// まずは入力チェック
			const auto& Joystick = j.Pin();
			if(Joystick->Input())
			{// 入力に合わせてイベントを飛ばす
				Joystick->Event(MessageHandler_);
			}
		}
	}
}

#include "HideWindowsPlatformTypes.h"

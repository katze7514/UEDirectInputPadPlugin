#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputJoystick.h"
#include "DirectInputPadDevice.h"

bool FDirectInputPadDevice::Init(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	MessageHandler_ = InMessageHandler;

	//static_cast<UGameEngine*>(GEngine)->GameViewportWindow;

	return true;
}

void FDirectInputPadDevice::SendControllerEvents()
{
}

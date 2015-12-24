#pragma once

#include "IDirectInputPadPlugin.h"

class FDirectInputPadDevice;

class FDirectInputPadPlugin : public IDirectInputPadPlugin
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** IInputDeviceModule immplementaion **/
	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);


public:
	TSharedPtr<FDirectInputPadDevice> GetDirectInputPadDevice();
};

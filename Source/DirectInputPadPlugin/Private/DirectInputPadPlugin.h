#pragma once

#include "../Public/IDirectInputPadPlugin.h"

class FDirectInputPadPlugin : public IDirectInputPadPlugin
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** IInputDeviceModule immplementaion **/
	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);
};

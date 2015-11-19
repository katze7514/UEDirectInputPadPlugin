#pragma once

#include "../Public/IDirectInputPlugin.h"

class FDirectInputPlugin : public IDirectInputPlugin
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** IInputDeviceModule immplementaion **/
	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);
};

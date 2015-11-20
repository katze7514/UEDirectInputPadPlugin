#pragma once

#include "InputCoreTypes.h"
#include "InputDevice.h"

class FDirectInputPadDevice : public IInputDevice
{
public:
	bool Init(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);

public:
	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents()override;

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)override{ MessageHandler_=InMessageHandler; }


private:
	TSharedRef<FGenericApplicationMessageHandler> MessageHandler_;
};

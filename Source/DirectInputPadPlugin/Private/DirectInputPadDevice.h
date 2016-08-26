#pragma once

#include "InputCoreTypes.h"
#include "InputDevice.h"

class FDirectInputDriver;
class FDirectInputJoystickFactory;
class FDirectInputJoystick;
class FXInputJoystickEmu;

class FDirectInputPadDevice : public IInputDevice
{
public:
	bool Init(bool bBackground=false);
	void Fin();

public:
	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents()override;

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)override{ MessageHandler_=InMessageHandler; }

public:
	virtual void Tick(float DeltaTime){}
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar){ return true; }
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value){}
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values){}

public:
	TWeakPtr<FXInputJoystickEmu>	GetJoystick(uint32 nPlayerID);
	int32							GetXInputDeviceNum()const{ return XInputDeviceNum_; }
	int32							GetDInputDeviceNum()const{ return DInputDeviceNum_; }

	bool							IsInitilized()const{ return bInit_; }

private:
	TSharedPtr<FGenericApplicationMessageHandler>	MessageHandler_;

	TSharedPtr<FDirectInputDriver>					DDriver_;
	TSharedPtr<FDirectInputJoystickFactory>			DFactory_;
	TArray<TSharedPtr<FXInputJoystickEmu>>			XJoysticks_; // 必ずMAX_JOYSITCKS分確保される。IndexがPlayerIndexと一致している

	int32											XInputDeviceNum_ = 0; // 有効なXInputデバイス数
	int32											DInputDeviceNum_ = 0; // 有効なDInputデバイス数

	bool											bInit_ = false; // 初期化フラグ
};

#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputPadPlugin.h"

#include "DirectInputPadDevice.h"
#include "DirectInputJoystick.h"
#include "DirectInputPadState.h"

#include "DirectInputPadJoystick.h"

void UDirectInputPadJoystick::SetJoysticks(const TWeakPtr<FDirectInputJoystick>& Joystick)
{
	Joystick_ = Joystick;
}


EXInputPadKeyNames	UDirectInputPadJoystick::GetKeyMap(EDirectInputPadKeyNames eDIKeys)
{
	return XIGamePad_LeftAnalogX;
}

void UDirectInputPadJoystick::SetKeyMap(EDirectInputPadKeyNames eDIKey, EXInputPadKeyNames eXIKey)
{
}

//////////////////////////////////
// UDirectInputPadFunctionLibrary
//////////////////////////////////
namespace{
TMap<int32, UDirectInputPadJoystick*> MapJoystick;
}


UDirectInputPadJoystick* UDirectInputPadFunctionLibrary::GetDirectInputPadJoystick(int32 PlayerID)
{
	auto Joy = MapJoystick.Find(PlayerID);
	if(Joy) return *Joy;

	auto& DIPadPlugin = static_cast<FDirectInputPadPlugin&>(IDirectInputPadPlugin::Get());
	const auto& DIDevice = DIPadPlugin.GetDirectInputPadDevice();

	auto DIPad = DIDevice->GetJoystick(PlayerID);
	if(DIPad.IsValid())
	{
		UDirectInputPadJoystick* pJoyPad = NewObject<UDirectInputPadJoystick>();
		pJoyPad->SetJoysticks(DIPad);
		MapJoystick.Emplace(PlayerID, pJoyPad);
		return pJoyPad;
	}

	return nullptr;
}

void UDirectInputPadFunctionLibrary::ClearDirectInputPadJoystickMap()
{
	MapJoystick.Empty();
}

#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputPadPlugin.h"
#include "DirectInputPadDevice.h"
#include "DirectInputJoystick.h"

#include "DirectInputPadJoystick.h"

DEFINE_LOG_CATEGORY_STATIC(DirectInputPadPlugin, Log, All)

namespace{
const FName XIGamepadKeyNames[] =
{
	FGamepadKeyNames::LeftAnalogX,              // XIGamePad_LeftAnalogX
	FGamepadKeyNames::LeftAnalogY,				// XIGamePad_LeftAnalogY
	FGamepadKeyNames::RightAnalogX,				// XIGamePad_RightAnalogX
	FGamepadKeyNames::RightAnalogY,				// XIGamePad_RightAnalogY
	FGamepadKeyNames::FaceButtonBottom,			// XIGamePad_Button_A
	FGamepadKeyNames::FaceButtonRight,			// XIGamePad_Button_B
	FGamepadKeyNames::FaceButtonLeft,			// XIGamePad_Button_X
	FGamepadKeyNames::FaceButtonTop,			// XIGamePad_Button_Y
	FGamepadKeyNames::LeftShoulder,				// XIGamePad_Button_LB
	FGamepadKeyNames::RightShoulder,			// XIGamePad_Button_RB
	FGamepadKeyNames::LeftTriggerThreshold,		// XIGamePad_Button_LTrigger
	FGamepadKeyNames::RightTriggerThreshold,	// XIGamePad_Button_RTrigger
	FGamepadKeyNames::SpecialLeft,				// XIGamePad_Button_BACK
	FGamepadKeyNames::SpecialRight,				// XIGamePad_Button_START
	FGamepadKeyNames::LeftThumb,				// XIGamePad_Button_LStick
	FGamepadKeyNames::RightThumb				// XIGamePad_Button_RStick
};

TMap<FName,EXInputPadKeyNames> XINameToEnumMap;

}

//////////////////////////////////
// UDirectInputPadJoystick
//////////////////////////////////
void UDirectInputPadJoystick::SetJoysticks(const TWeakPtr<FDirectInputJoystick>& Joystick)
{
	Joystick_ = Joystick;
}

EXInputPadKeyNames	UDirectInputPadJoystick::GetKeyMap(EDirectInputPadKeyNames DIKey)
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			return XINameToEnumMap.FindRef(Joy->GetXIKey(DIKey));
		}
	}
	return XIGamePad_END;
}

void UDirectInputPadJoystick::SetKeyMap(EDirectInputPadKeyNames DIKey, EXInputPadKeyNames XIKey)
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			if(XIKey==XIGamePad_END)
			{	Joy->SetXIKey(DIKey, FName());	}
			else
			{	Joy->SetXIKey(DIKey, XIGamepadKeyNames[XIKey]);	}
		}
	}
}

bool UDirectInputPadJoystick::IsAxisReverse(EDirectInputPadKeyNames DIAxis)
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			return Joy->IsAxisReverse(DIAxis);
		}
	}

	return false;
}

void UDirectInputPadJoystick::SetAxisReverse(EDirectInputPadKeyNames DIAxis, bool bReverse)
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			Joy->SetAxisReverse(DIAxis, bReverse);
		}
	}
}

FString UDirectInputPadJoystick::GetProductName()
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			DIDEVICEINSTANCE inst;
			inst.dwSize = sizeof(DIDEVICEINSTANCE);
			if(Joy->pDevice_->GetDeviceInfo(&inst)==DI_OK)
			{
				return FString(inst.tszProductName);
			}
		}
	}

	return "";
}

FString	UDirectInputPadJoystick::GetGUID()
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			DIDEVICEINSTANCE inst;
			inst.dwSize = sizeof(DIDEVICEINSTANCE);
			if(Joy->pDevice_->GetDeviceInfo(&inst)==DI_OK)
			{
				LPOLESTR p;
				if(FAILED(StringFromCLSID(inst.guidInstance,&p)))
					return "";

				FString s(p);
				CoTaskMemFree(p);
				return s;
			}
		}
	}

	return "";
}

FDIGamePadKeyState UDirectInputPadJoystick::GetChangedKeyState()
{
	FDIGamePadKeyState state;

	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			if(Joy->X()!=0)
			{
				state.KeyName = DIGamePad_AXIS_X;
				state.Value   = Joy->X();
				return state;
			}

			if(Joy->Y()!=0)
			{
				state.KeyName = DIGamePad_AXIS_Y;
				state.Value   = Joy->Y();
				return state;
			}

			if(Joy->Z()!=0)
			{
				state.KeyName = DIGamePad_AXIS_Z;
				state.Value   = Joy->Z();
				return state;
			}

			if(Joy->RotX()!=0)
			{
				state.KeyName = DIGamePad_ROT_X;
				state.Value   = Joy->RotX();
				return state;
			}

			if(Joy->RotY()!=0)
			{
				state.KeyName = DIGamePad_ROT_Y;
				state.Value   = Joy->RotY();
				return state;
			}

			if(Joy->RotZ()!=0)
			{
				state.KeyName = DIGamePad_ROT_Z;
				state.Value   = Joy->RotZ();
				return state;
			}

			for(uint8 i=0; i<32; ++i)
			{
				if(Joy->IsPush(i))
				{
					state.KeyName = TEnumAsByte<EDirectInputPadKeyNames>(DIGamePad_Button1 + i);
					state.Value   = 1.0f;
					return state;
				}
				else if(Joy->IsRelease(i))
				{
					state.KeyName = TEnumAsByte<EDirectInputPadKeyNames>(DIGamePad_Button1 + i);
					state.Value   = -1.0f;
					return state;
				}
			}
		}
	}
	return state; 
}


TArray<FDIGamePadKeyState> UDirectInputPadJoystick::GetAllChangedKeyState()
{
	TArray<FDIGamePadKeyState> aState;

	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			FDIGamePadKeyState state;

			if(Joy->X()!=0)
			{
				state.KeyName = DIGamePad_AXIS_X;
				state.Value   = Joy->X();
				aState.Add(state);
			}

			if(Joy->Y()!=0)
			{
				state.KeyName = DIGamePad_AXIS_Y;
				state.Value   = Joy->Y();
				aState.Add(state);
			}

			if(Joy->Z()!=0)
			{
				state.KeyName = DIGamePad_AXIS_Z;
				state.Value   = Joy->Z();
				aState.Add(state);
			}

			if(Joy->RotX()!=0)
			{
				state.KeyName = DIGamePad_ROT_X;
				state.Value   = Joy->RotX();
				aState.Add(state);
			}

			if(Joy->RotY()!=0)
			{
				state.KeyName = DIGamePad_ROT_Y;
				state.Value   = Joy->RotY();
				aState.Add(state);
			}

			if(Joy->RotZ()!=0)
			{
				state.KeyName = DIGamePad_ROT_Z;
				state.Value   = Joy->RotZ();
				aState.Add(state);
			}

			for(uint8 i=0; i<32; ++i)
			{
				if(Joy->IsPush(i))
				{
					state.KeyName = TEnumAsByte<EDirectInputPadKeyNames>(DIGamePad_Button1 + i);
					state.Value   = 1.0f;
					aState.Add(state);
				}
				else if(Joy->IsRelease(i))
				{
					state.KeyName = TEnumAsByte<EDirectInputPadKeyNames>(DIGamePad_Button1 + i);
					state.Value   = -1.0f;
					aState.Add(state);
				}
			}
		}
	}

	aState.Shrink();

	return std::move(aState); 
}

void UDirectInputPadJoystick::ClearInput()
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			Joy->ClearBuf();
		}
	}
}

//////////////////////////////////
// UDirectInputPadFunctionLibrary
//////////////////////////////////
namespace{
TArray<TWeakObjectPtr<UDirectInputPadJoystick>> ArrayJoysticks;

inline TSharedPtr<FDirectInputPadDevice> GetDirectInputPadDevice()
{
	auto& DIPadPlugin = static_cast<FDirectInputPadPlugin&>(IDirectInputPadPlugin::Get());
	return DIPadPlugin.GetDirectInputPadDevice();
}
} // namespace

int32 UDirectInputPadFunctionLibrary::GetXInputPadNum()
{
	const auto& DIDevice = GetDirectInputPadDevice();
	return DIDevice->GetXInputDeviceNum();
}

int32 UDirectInputPadFunctionLibrary::GetDirectInputPadNum()
{
	const auto& DIDevice = GetDirectInputPadDevice();
	return DIDevice->GetDInputDeviceNum();
}

UDirectInputPadJoystick* UDirectInputPadFunctionLibrary::GetDirectInputPadJoystick(int32 PlayerIndex)
{
	if(PlayerIndex>=0 && PlayerIndex<=7)
	{
		auto Joy = ArrayJoysticks[PlayerIndex];
		if(Joy.IsValid()) return Joy.Get();

		const auto& DIDevice = GetDirectInputPadDevice();

		auto DIPad = DIDevice->GetJoystick(PlayerIndex);
		if(DIPad.IsValid())
		{
			UDirectInputPadJoystick* pJoyPad = NewObject<UDirectInputPadJoystick>();
			pJoyPad->SetJoysticks(DIPad);
			ArrayJoysticks[PlayerIndex] = pJoyPad;
			return pJoyPad;
		}
	}

	UE_LOG(DirectInputPadPlugin, Warning, TEXT("Not Found PlayerIndex(%d) DirectInputJoystick."), PlayerIndex);
	return nullptr;
}

void UDirectInputPadFunctionLibrary::InitDirectInputPadJoystickLibrary()
{
	XINameToEnumMap.Emplace(FGamepadKeyNames::LeftAnalogX,			XIGamePad_LeftAnalogX);
	XINameToEnumMap.Emplace(FGamepadKeyNames::LeftAnalogY,			XIGamePad_LeftAnalogY);
	XINameToEnumMap.Emplace(FGamepadKeyNames::RightAnalogX,			XIGamePad_RightAnalogX);
	XINameToEnumMap.Emplace(FGamepadKeyNames::RightAnalogY,			XIGamePad_RightAnalogY);
	XINameToEnumMap.Emplace(FGamepadKeyNames::FaceButtonBottom,		XIGamePad_Button_A);
	XINameToEnumMap.Emplace(FGamepadKeyNames::FaceButtonRight,		XIGamePad_Button_B);
	XINameToEnumMap.Emplace(FGamepadKeyNames::FaceButtonLeft,		XIGamePad_Button_X);
	XINameToEnumMap.Emplace(FGamepadKeyNames::FaceButtonTop,		XIGamePad_Button_Y);
	XINameToEnumMap.Emplace(FGamepadKeyNames::LeftShoulder,			XIGamePad_Button_LB);
	XINameToEnumMap.Emplace(FGamepadKeyNames::RightShoulder,		XIGamePad_Button_RB);
	XINameToEnumMap.Emplace(FGamepadKeyNames::LeftTriggerThreshold,	XIGamePad_Button_LTrigger);
	XINameToEnumMap.Emplace(FGamepadKeyNames::RightTriggerThreshold,XIGamePad_Button_RTrigger);
	XINameToEnumMap.Emplace(FGamepadKeyNames::SpecialLeft,			XIGamePad_Button_BACK);
	XINameToEnumMap.Emplace(FGamepadKeyNames::SpecialRight,			XIGamePad_Button_START);
	XINameToEnumMap.Emplace(FGamepadKeyNames::LeftThumb,			XIGamePad_Button_LStick);
	XINameToEnumMap.Emplace(FGamepadKeyNames::RightThumb,			XIGamePad_Button_RStick);
	XINameToEnumMap.Emplace(FName(""),								XIGamePad_END);

	XINameToEnumMap.Shrink();

	ArrayJoysticks.SetNum(8);
}

void UDirectInputPadFunctionLibrary::FinDirectInputPadJoystickLibrary()
{
	ArrayJoysticks.Empty();
}

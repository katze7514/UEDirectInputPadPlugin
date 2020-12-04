#include "DirectInputPadJoystick.h"
#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputPadSaveGame.h"

#include "DirectInputPadPlugin.h"
#include "DirectInputPadDevice.h"
#include "DirectInputJoystick.h"

//////////////////////////////////
// UDirectInputPadJoystick
//////////////////////////////////

TArray<FDIKeyMapInfo> UDirectInputPadJoystick::GetKeyMap(EXInputPadKeyNames eXIKey)
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			return Joy->GetKeyMap(eXIKey);
		}
	}

	TArray<FDIKeyMapInfo> map;
	map.SetNum(2);
	return std::move(map);
}

void UDirectInputPadJoystick::SetKeyMap(EXInputPadKeyNames XIKey, EDirectInputPadKeyNames DIKey, bool bNegative)
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			Joy->SetKeyMap(XIKey, FDIKeyMapInfo(DIKey,bNegative));
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
			return Joy->GetDIPad()->IsAxisReverse(DIAxis);
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
			Joy->GetDIPad()->SetAxisReverse(DIAxis, bReverse);
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
			if(Joy->GetDIPad()->pDevice_->GetDeviceInfo(&inst)==DI_OK)
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
			if(Joy->GetDIPad()->pDevice_->GetDeviceInfo(&inst)==DI_OK)
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

namespace{
	struct SwapReverseFlag
	{
		SwapReverseFlag(TSharedPtr<FDirectInputJoystick> Joy, EDirectInputPadKeyNames eKey, bool bReal):Joy_(Joy),eKey_(eKey),bReal_(bReal)
		{
			if(bReal_)
			{
				bFlag_ = Joy_->IsAxisReverse(eKey_);
				Joy_->SetAxisReverse(eKey_, false);
			}
		}

		~SwapReverseFlag()
		{
			if(bReal_){ Joy_->SetAxisReverse(eKey_, bFlag_); }
		}

		TSharedPtr<FDirectInputJoystick>& Joy_;
		EDirectInputPadKeyNames eKey_;
		bool bReal_;
		bool bFlag_;
	};
}

bool UDirectInputPadJoystick::IsChagnedKeyState()const
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			return Joy->GetDIPad()->IsChangedKeyState();
		}
	}

	return false;
}

FDIGamePadKeyState UDirectInputPadJoystick::GetChangedKeyState(bool bReal, bool bBtn)
{	
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			const auto& DIPad = Joy->GetDIPad();

			auto CheckJoyState = [this, &DIPad, bReal]()
			{
				FDIGamePadKeyState state;
				{
					SwapReverseFlag srf(DIPad, DIGamePad_AXIS_X, bReal);
					if(DIPad->X()!=DIPad->InitX())
					{
						state.KeyName = DIGamePad_AXIS_X;
						state.Value   = DIPad->X();
						return state;
					}
				}

				{
					SwapReverseFlag srf(DIPad, DIGamePad_AXIS_Y, bReal);
					if(DIPad->Y()!=DIPad->InitY())
					{
						state.KeyName = DIGamePad_AXIS_Y;
						state.Value   = DIPad->Y();
						return state;
					}
				}

				{
					SwapReverseFlag srf(DIPad, DIGamePad_AXIS_Z, bReal);
					if(DIPad->Z()!=DIPad->InitZ())
					{
						state.KeyName = DIGamePad_AXIS_Z;
						state.Value   = DIPad->Z();
						return state;
					}
				}

				{
					SwapReverseFlag srf(DIPad, DIGamePad_ROT_X, bReal);

					if(DIPad->RotX()!=DIPad->InitRotX())
					{
						state.KeyName = DIGamePad_ROT_X;
						state.Value   = DIPad->RotX();
						return state;
					}
				}

				{
					SwapReverseFlag srf(DIPad, DIGamePad_ROT_Y, bReal);
					if(DIPad->RotY()!=DIPad->InitRotY())
					{
						state.KeyName = DIGamePad_ROT_Y;
						state.Value   = DIPad->RotY();
						return state;
					}
				}

				{
					SwapReverseFlag srf(DIPad, DIGamePad_ROT_Z, bReal);
					if(DIPad->RotZ()!=DIPad->InitRotZ())
					{
						state.KeyName = DIGamePad_ROT_Z;
						state.Value   = DIPad->RotZ();
						return state;
					}
				}

				return state;
			};

			auto CheckBtnState = [this, &DIPad, bReal]()
			{
				FDIGamePadKeyState state;

				for(uint8 i=0; i<=36; ++i)
				{
					if(DIPad->IsPush(i))
					{
						state.KeyName = TEnumAsByte<EDirectInputPadKeyNames>(DIGamePad_Button1 + i);
						state.Value   = 1.0f;
						return state;
					}
					else if(DIPad->IsRelease(i))
					{
						state.KeyName = TEnumAsByte<EDirectInputPadKeyNames>(DIGamePad_Button1 + i);
						state.Value   = -1.0f;
						return state;
					}
				}

				return state;
			};

			if(bBtn)
			{
				auto state = CheckBtnState();
				if(state.KeyName!=DIGamePad_END)
				{
					//UE_LOG(LogDirectInputPadPlugin, Log, TEXT("ChangedState %d"), state.KeyName);
					return state;
				}
				return CheckJoyState();
			}
			else
			{
				auto state = CheckJoyState();
				if(state.KeyName!=DIGamePad_END) return state;
				return CheckBtnState();
			}
		}
	}

	return FDIGamePadKeyState();
}

TArray<FDIGamePadKeyState> UDirectInputPadJoystick::GetAllChangedKeyState(bool bReal)
{
	TArray<FDIGamePadKeyState> aState;

	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			const auto& DIPad =Joy->GetDIPad();

			FDIGamePadKeyState state;

			{
				SwapReverseFlag srf(DIPad, DIGamePad_AXIS_X, bReal);
				if(DIPad->X()!=DIPad->InitX())
				{
					state.KeyName = DIGamePad_AXIS_X;
					state.Value   = DIPad->X();
					aState.Add(state);
				}
			}

			{
				SwapReverseFlag srf(DIPad, DIGamePad_AXIS_Y, bReal);
				if(DIPad->Y()!=DIPad->InitY())
				{
					state.KeyName = DIGamePad_AXIS_Y;
					state.Value   = DIPad->Y();
					aState.Add(state);
				}
			}

			{
				SwapReverseFlag srf(DIPad, DIGamePad_AXIS_Z, bReal);
				if(DIPad->Z()!=DIPad->InitZ())
				{
					state.KeyName = DIGamePad_AXIS_Z;
					state.Value   = DIPad->Z();
					aState.Add(state);
				}
			}

			{
				SwapReverseFlag srf(DIPad, DIGamePad_ROT_X, bReal);
				if(DIPad->RotX()!=DIPad->InitRotX())
				{
					state.KeyName = DIGamePad_ROT_X;
					state.Value   = DIPad->RotX();
					aState.Add(state);	
				}
			}

			{
				SwapReverseFlag srf(DIPad, DIGamePad_ROT_Y, bReal);
				if(DIPad->RotY()!=DIPad->InitRotY())
				{
					state.KeyName = DIGamePad_ROT_Y;
					state.Value   = DIPad->RotY();
					aState.Add(state);
				}
			}

			{
				SwapReverseFlag srf(DIPad, DIGamePad_ROT_Z, bReal);
				if(DIPad->RotZ()!=DIPad->InitRotZ())
				{
					state.KeyName = DIGamePad_ROT_Z;
					state.Value   = DIPad->RotZ();
					aState.Add(state);
				}
			}

			for(uint8 i=0; i<32; ++i)
			{
				if(DIPad->IsPush(i))
				{
					state.KeyName = TEnumAsByte<EDirectInputPadKeyNames>(DIGamePad_Button1 + i);
					state.Value   = 1.0f;
					aState.Add(state);
				}
				else if(DIPad->IsRelease(i))
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
			Joy->GetDIPad()->ClearBuf();
		}
	}
}

int32 UDirectInputPadJoystick::GetPlayerIndex()const
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			return Joy->GetDIPad()->GetPlayerIndex();
		}
	}

	return -1;
}

/*void UDirectInputPadJoystick::SetPlayerIndex(int32 PlayerIndex)
{
	if(Joystick_.IsValid())
	{
		const auto& Joy = Joystick_.Pin();
		if(Joy.IsValid())
		{
			Joy->GetDIPad()->SetPlayerIndex(PlayerIndex);
		}
	}
}*/

void UDirectInputPadJoystick::SetJoysticks(const TWeakPtr<FXInputJoystickEmu>& Joystick)
{
	Joystick_ = Joystick;
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

bool UDirectInputPadFunctionLibrary::IsInitilizedDirectInputPadPlugin()
{
	const auto& DIDevice = GetDirectInputPadDevice();
	if(DIDevice.IsValid())
		return DIDevice->IsInitilized();
	else
		return false;
}

int32 UDirectInputPadFunctionLibrary::GetXInputPadNum()
{
	const auto& DIDevice = GetDirectInputPadDevice();
	if(DIDevice.IsValid())
		return DIDevice->GetXInputDeviceNum();
	else
		return -1;
}

int32 UDirectInputPadFunctionLibrary::GetDirectInputPadNum()
{
	const auto& DIDevice = GetDirectInputPadDevice();

	if(DIDevice.IsValid())
		return DIDevice->GetDInputDeviceNum();
	else
		return -1;
}

UDirectInputPadJoystick* UDirectInputPadFunctionLibrary::GetDirectInputPadJoystick(int32 PlayerIndex)
{
	if(ArrayJoysticks.IsValidIndex(PlayerIndex))
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

	UE_LOG(LogDirectInputPadPlugin, Warning, TEXT("Not Found PlayerIndex(%d) DirectInputJoystick."), PlayerIndex);
	return nullptr;
}

bool UDirectInputPadFunctionLibrary::SaveDirectInputPadKeyMap(const FString& SlotName, int32 UserIndex)
{
	if(SlotName.IsEmpty()||UserIndex<0||GetDirectInputPadNum()<=0) return false;

	UDirectInputPadSaveGame* SaveGame = Cast<UDirectInputPadSaveGame>(UGameplayStatics::CreateSaveGameObject(UDirectInputPadSaveGame::StaticClass()));
	if(SaveGame==nullptr) return false;

	for(int32 i=0; i<GetDirectInputPadNum(); ++i)
	{
		auto Joy = GetDirectInputPadJoystick(i);
		if(Joy==nullptr) continue;

		FDIPadKeyMaps KeyMaps;
		KeyMaps.GUID = Joy->GetGUID();
		// 軸
		for(uint8 j=XIGamePad_LeftAnalogX; j<=XIGamePad_RTAnalog; ++j)
		{
			TArray<FDIKeyMapInfo> map;
			map = Joy->GetKeyMap(TEnumAsByte<EXInputPadKeyNames>(j));
			KeyMaps.aMapInfo_.Add(map[0]);
			KeyMaps.aMapInfo_.Add(map[1]);
		}

		// ボタン
		for(uint8 j=XIGamePad_Button_A; j<=XIGamePad_DPad_Left; ++j)
		{
			TArray<FDIKeyMapInfo> map;
			map = Joy->GetKeyMap(TEnumAsByte<EXInputPadKeyNames>(j));
			KeyMaps.aMapInfo_.Add(map[0]);
		}
		SaveGame->DIPadKeyMaps.Add(KeyMaps);
	}

	if(SaveGame->DIPadKeyMaps.Num()<=0) return false;

	return UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
}

bool UDirectInputPadFunctionLibrary::LoadDirectInputPadKeyMap(const FString& SlotName, int32 UserIndex)
{
	auto save = UGameplayStatics::LoadGameFromSlot(SlotName,UserIndex);
	if(save==nullptr) return false;

	UDirectInputPadSaveGame* SaveGame = Cast<UDirectInputPadSaveGame>(save);
	if(SaveGame==nullptr) return false;

	for(auto& KeyMaps : SaveGame->DIPadKeyMaps)
	{
		for(int32 i=0; i<GetDirectInputPadNum(); ++i)
		{
			auto Joy = GetDirectInputPadJoystick(i);
			if(Joy==nullptr) continue;
			if(KeyMaps.GUID==Joy->GetGUID())
			{
				for(uint8 eXIKey = XIGamePad_LeftAnalogX; eXIKey<=XIGamePad_RTAnalog; ++eXIKey)
				{
					auto& Info = KeyMaps.aMapInfo_[eXIKey*2];
					Joy->SetKeyMap(TEnumAsByte<EXInputPadKeyNames>(eXIKey), Info.eDIKey_, Info.bNegative_);

					auto& Info2 = KeyMaps.aMapInfo_[eXIKey*2+1];
					if(Info2.eDIKey_!=DIGamePad_END)
						Joy->SetKeyMap(TEnumAsByte<EXInputPadKeyNames>(eXIKey), Info2.eDIKey_, Info2.bNegative_);
				}

				uint8 btn = XIGamePad_RTAnalog*2 + 1 + 1;
				for(uint8 eXIKey = XIGamePad_Button_A; eXIKey<=XIGamePad_DPad_Left; ++eXIKey)
				{
					auto& Info = KeyMaps.aMapInfo_[btn++];
					Joy->SetKeyMap(TEnumAsByte<EXInputPadKeyNames>(eXIKey), Info.eDIKey_, Info.bNegative_);
				}
			}
		}
	}

	return true;
}

void UDirectInputPadFunctionLibrary::InitDirectInputPadJoystickLibrary()
{
	ArrayJoysticks.SetNum(8);
}

void UDirectInputPadFunctionLibrary::FinDirectInputPadJoystickLibrary()
{
	ArrayJoysticks.Empty();
}

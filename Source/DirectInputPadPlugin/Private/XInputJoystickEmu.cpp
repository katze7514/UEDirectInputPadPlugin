#include "XInputJoystickEmu.h"
#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputJoystick.h"

namespace{
const float AXIS_THRESHOLD = 0.6f;
}

void FXInputJoystickEmu::Init(TSharedRef<FDirectInputJoystick> DIPad)
{
	DIPad_ = DIPad;

	SetAnalogMap(XIGamePad_LeftAnalogX,		DIGamePad_AXIS_X);
	SetAnalogMap(XIGamePad_LeftAnalogY,		DIGamePad_AXIS_Y);
	
	SetAnalogMap(XIGamePad_RightAnalogX,	DIGamePad_AXIS_Z);
	SetAnalogMap(XIGamePad_RightAnalogY,	DIGamePad_ROT_Z);

	SetAnalogMap(XIGamePad_LTAnalog,		DIGamePad_Button7);
	SetAnalogMap(XIGamePad_RTAnalog,		DIGamePad_Button8);

	SetButtonMap(XIGamePad_DPad_Up,			DIGamePad_POV_Up);
	SetButtonMap(XIGamePad_DPad_Down,		DIGamePad_POV_Down);
	SetButtonMap(XIGamePad_DPad_Right,		DIGamePad_POV_Right);
	SetButtonMap(XIGamePad_DPad_Left,		DIGamePad_POV_Left);

	SetButtonMap(XIGamePad_Button_A,		DIGamePad_Button1);
	SetButtonMap(XIGamePad_Button_B,		DIGamePad_Button2);
	SetButtonMap(XIGamePad_Button_X,		DIGamePad_Button3);
	SetButtonMap(XIGamePad_Button_Y,		DIGamePad_Button4);

	SetButtonMap(XIGamePad_Button_LB,		DIGamePad_Button5);
	SetButtonMap(XIGamePad_Button_RB,		DIGamePad_Button6);

	SetButtonMap(XIGamePad_Button_BACK,		DIGamePad_Button9);
	SetButtonMap(XIGamePad_Button_START,	DIGamePad_Button10);

	SetButtonMap(XIGamePad_Button_LStick,	DIGamePad_Button11);
	SetButtonMap(XIGamePad_Button_RStick,	DIGamePad_Button12);
}

void FXInputJoystickEmu::SetKeyMap(EXInputPadKeyNames eXIKey, FDIKeyMapInfo DIKeyInfo)
{
	//if(DIKeyInfo.eDIKey_ == DIGamePad_END)
	//UE_LOG(LogDirectInputPadPlugin, Warning, TEXT("SetKeyMap: %d %d"), static_cast<int>(eXIKey), static_cast<int>(DIKeyInfo.eDIKey_));

	SetAnalogMap(eXIKey, DIKeyInfo);
	SetButtonMap(eXIKey, DIKeyInfo);
}

void FXInputJoystickEmu::SetAnalogMap(EXInputPadKeyNames eXIKey, FDIKeyMapInfo DIKeyInfo)
{
	const EDirectInputPadKeyNames eDIKey = DIKeyInfo.eDIKey_;
	const bool bNegative				 = DIKeyInfo.bNegative_;

	if(eDIKey<=DIGamePad_ROT_Z)
	{// Ž˛‚đŽ˛‚É
		switch(eXIKey)
		{
		case XIGamePad_LeftAnalogX:
			SetAnalogMapAnalog(eDIKey, LeftAnalogX_, LeftStickRight_, LeftStickLeft_);
		break;

		case XIGamePad_LeftAnalogY:
			SetAnalogMapAnalog(eDIKey, LeftAnalogY_, LeftStickUp_, LeftStickDown_);
		break;

		case XIGamePad_RightAnalogX:
			SetAnalogMapAnalog(eDIKey, RightAnalogX_, RightStickRight_, RightStickLeft_);
		break;

		case XIGamePad_RightAnalogY:
			SetAnalogMapAnalog(eDIKey, RightAnalogY_, RightStickUp_, RightStickDown_);
		break;

		case XIGamePad_LTAnalog:
		{
			BtnDelegate b;
			SetAnalogMapAnalog(eDIKey, LeftTriggerAnalog_, LeftTrigger_, b);
		}
		break;

		case XIGamePad_RTAnalog:
		{
			BtnDelegate b;
			SetAnalogMapAnalog(eDIKey, RightTriggerAnalog_, RightTrigger_, b);
		}
		break;
		}
	}
	else
	{// {^“‚đŽ˛‚É
		switch(eXIKey)
		{
		case XIGamePad_LeftAnalogX:
			SetAnalogMapBtn(eDIKey, LeftAnalogX_, bNegative ? LeftStickLeft_ : LeftStickRight_, bNegative);
		break;

		case XIGamePad_LeftAnalogY:
			SetAnalogMapBtn(eDIKey, LeftAnalogY_, bNegative ? LeftStickDown_ : LeftStickUp_, bNegative);
		break;

		case XIGamePad_RightAnalogX:
			SetAnalogMapBtn(eDIKey, RightAnalogX_, bNegative ? RightStickLeft_ : RightStickRight_, bNegative);
		break;

		case XIGamePad_RightAnalogY:
			SetAnalogMapBtn(eDIKey, RightAnalogY_, bNegative ? RightStickDown_ : RightStickUp_, bNegative);
		break;

		case XIGamePad_LTAnalog:
			SetAnalogMapBtn(eDIKey, LeftTriggerAnalog_, LeftTrigger_, false);
		break;

		case XIGamePad_RTAnalog:
			SetAnalogMapBtn(eDIKey, RightTriggerAnalog_, RightTrigger_, false);
		break;
		}
	}
}

namespace{
inline void SetBtnDelegate(BtnDelegate& Btn, EDirectInputPadKeyNames eDIKey, const TArray<std::function<bool()>>& aBtnDele, bool bNegative=false)
{
	Btn.DIKey_.eDIKey_		= eDIKey;
	Btn.DIKey_.bNegative_	= bNegative;
	Btn.IsPress_			= aBtnDele[0];
	Btn.IsPush_				= aBtnDele[1];
	Btn.IsRelease_			= aBtnDele[2];
}
}
void FXInputJoystickEmu::SetAnalogMapAnalog(EDirectInputPadKeyNames eDIKey, AnalogDelegate& Analog, BtnDelegate& Pos, BtnDelegate& Neg)
{
	Analog.DIKeyAxis_.eDIKey_	= eDIKey;
	Analog.DIKeyAxis_.bNegative_= false;
	Analog.Axis_				= CreateAnalogDelegate(eDIKey);

	Analog.DIKeyAxisNegative_.eDIKey_	= DIGamePad_END;
	Analog.DIKeyAxisNegative_.bNegative_= true;
	Analog.AxisNegative_				= nullptr;

	SetBtnDelegate(Pos, eDIKey, CreateButtonDelegate(eDIKey, false));
	SetBtnDelegate(Neg, eDIKey, CreateButtonDelegate(eDIKey, true), true);
}

void FXInputJoystickEmu::SetAnalogMapBtn(EDirectInputPadKeyNames eDIKey, AnalogDelegate& Analog, BtnDelegate& Btn, bool bNegative)
{
	if(bNegative)
	{
		Analog.DIKeyAxisNegative_.eDIKey_		= eDIKey;
		Analog.DIKeyAxisNegative_.bNegative_	= true;
		Analog.AxisNegative_					= CreateAnalogDelegate(eDIKey,true);
	}
	else
	{
		Analog.DIKeyAxis_.eDIKey_		= eDIKey;
		Analog.DIKeyAxis_.bNegative_	= false;
		Analog.Axis_					= CreateAnalogDelegate(eDIKey,false);
	}

	SetBtnDelegate(Btn, eDIKey, CreateButtonDelegate(eDIKey));
}

void FXInputJoystickEmu::SetButtonMap(EXInputPadKeyNames eXIKey, FDIKeyMapInfo DIKeyInfo)
{
	const EDirectInputPadKeyNames eDIKey = DIKeyInfo.eDIKey_;
	const bool bNegative				 = DIKeyInfo.bNegative_;

	TArray<std::function<bool()>> aBtnDele;

	switch(eXIKey)
	{
	case XIGamePad_Button_A:
		SetBtnDelegate(BtnA_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_B:
		SetBtnDelegate(BtnB_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_X:
		SetBtnDelegate(BtnX_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_Y:
		SetBtnDelegate(BtnY_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_LB:
		SetBtnDelegate(BtnLB_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_RB:
		SetBtnDelegate(BtnRB_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_LT:
		SetBtnDelegate(LeftTrigger_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_RT:
		SetBtnDelegate(RightTrigger_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_BACK:
		SetBtnDelegate(BtnBack_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_START:
		SetBtnDelegate(BtnStart_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_LStick:
		SetBtnDelegate(BtnLStick_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_RStick:
		SetBtnDelegate(BtnRStick_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_DPad_Up:
		SetBtnDelegate(DPadUp_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_DPad_Down:
		SetBtnDelegate(DPadDown_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_DPad_Right:
		SetBtnDelegate(DPadRight_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_DPad_Left:
		SetBtnDelegate(DPadLeft_, eDIKey, CreateButtonDelegate(eDIKey, bNegative));
	break; 
	}
}

TArray<FDIKeyMapInfo> FXInputJoystickEmu::GetKeyMap(EXInputPadKeyNames eXIKey)
{
	TArray<FDIKeyMapInfo> map;
	map.SetNum(2);

	map[0] = GetButtonDIKeyMapInfo(eXIKey);
	if(map[0].eDIKey_==DIGamePad_END)
	{
		map = GetAnalogDIKeyMapInfo(eXIKey);
	}
	return std::move(map);
}

TArray<FDIKeyMapInfo> FXInputJoystickEmu::GetAnalogDIKeyMapInfo(EXInputPadKeyNames eXIKey)const
{
	TArray<FDIKeyMapInfo> map;
	map.SetNum(2);

	switch(eXIKey)
	{
	case XIGamePad_LeftAnalogX:
		map[0] = LeftAnalogX_.DIKeyAxis_;
		map[1] = LeftAnalogX_.DIKeyAxisNegative_;
	break;

	case XIGamePad_LeftAnalogY:
		map[0] = LeftAnalogY_.DIKeyAxis_;
		map[1] = LeftAnalogY_.DIKeyAxisNegative_;
	break;

	case XIGamePad_RightAnalogX:
		map[0] = RightAnalogX_.DIKeyAxis_;
		map[1] = RightAnalogX_.DIKeyAxisNegative_;
	break;

	case XIGamePad_RightAnalogY:
		map[0] = RightAnalogY_.DIKeyAxis_;
		map[1] = RightAnalogY_.DIKeyAxisNegative_;
	break;

	case XIGamePad_LTAnalog:
		map[0] = LeftTriggerAnalog_.DIKeyAxis_;
		map[1] = LeftTriggerAnalog_.DIKeyAxisNegative_;
	break;

	case XIGamePad_RTAnalog:
		map[0] = RightTriggerAnalog_.DIKeyAxis_;
		map[1] = RightTriggerAnalog_.DIKeyAxisNegative_;
	break;
	}

	return std::move(map);
};

FDIKeyMapInfo FXInputJoystickEmu::GetButtonDIKeyMapInfo(EXInputPadKeyNames eXIKey)const
{
	switch(eXIKey)
	{
	case XIGamePad_Button_A:		return BtnA_.DIKey_;
	case XIGamePad_Button_B:		return BtnB_.DIKey_;
	case XIGamePad_Button_X:		return BtnX_.DIKey_;
	case XIGamePad_Button_Y:		return BtnY_.DIKey_;
	case XIGamePad_Button_LB:		return BtnLB_.DIKey_;
	case XIGamePad_Button_RB:		return BtnRB_.DIKey_;
	case XIGamePad_Button_LT:		return LeftTrigger_.DIKey_;
	case XIGamePad_Button_RT:		return RightTrigger_.DIKey_;
	case XIGamePad_Button_BACK:		return BtnBack_.DIKey_;
	case XIGamePad_Button_START:	return BtnStart_.DIKey_;
	case XIGamePad_Button_LStick:	return BtnLStick_.DIKey_;
	case XIGamePad_Button_RStick:	return BtnRStick_.DIKey_;
	case XIGamePad_DPad_Up:			return DPadUp_.DIKey_;
	case XIGamePad_DPad_Down:		return DPadDown_.DIKey_;
	case XIGamePad_DPad_Right:		return DPadRight_.DIKey_;
	case XIGamePad_DPad_Left:		return DPadLeft_.DIKey_;
	}

	return DIGamePad_END;
};

std::function<float()> FXInputJoystickEmu::CreateAnalogDelegate(EDirectInputPadKeyNames eDIKey, bool bNegative)
{
	switch(eDIKey)
	{
	case DIGamePad_AXIS_X:
		return [this](){ return DIPad_->X(); };
	break;

	case DIGamePad_AXIS_Y:
		return [this](){ return DIPad_->Y(); };
	break;

	case DIGamePad_AXIS_Z:
		return [this](){ return DIPad_->Z(); };
	break;

	case DIGamePad_ROT_X:
		return [this](){ return DIPad_->RotX(); };
	break;

	case DIGamePad_ROT_Y:
		return [this](){ return DIPad_->RotY(); };
	break;

	case DIGamePad_ROT_Z:
		return [this](){ return DIPad_->RotZ(); };
	break;

	case DIGamePad_END:
		return nullptr;
	break;

	default:
		if(bNegative)
			return [this,eDIKey](){ return DIPad_->IsPress(eDIKey-DIGamePad_Button1) ? -1.0f : 0.0f; };
		else
			return [this,eDIKey](){ return DIPad_->IsPress(eDIKey-DIGamePad_Button1) ? 1.0f : 0.0f; };
	break;
	}

	return nullptr;
}

TArray<std::function<bool()>> FXInputJoystickEmu::CreateButtonDelegate(EDirectInputPadKeyNames eDIKey, bool bNegative)
{
	TArray<std::function<bool()>> aDele;

	switch(eDIKey)
	{
	case DIGamePad_AXIS_X:
		if(bNegative)
		{
			aDele.Add([this](){ return  DIPad_->X() <  -AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->X() <  -AXIS_THRESHOLD)  && (DIPad_->PrevX() >= -AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->X() >= -AXIS_THRESHOLD)  && (DIPad_->PrevX() <  -AXIS_THRESHOLD); });
		}
		else
		{
			aDele.Add([this](){ return  DIPad_->X() >  AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->X() >  AXIS_THRESHOLD)  && (DIPad_->PrevX() <= AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->X() <= AXIS_THRESHOLD)  && (DIPad_->PrevX() >  AXIS_THRESHOLD); });
		}
	break;

	case DIGamePad_AXIS_Y:
		if(bNegative)
		{
			aDele.Add([this](){ return  DIPad_->Y() <  -AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->Y() <  -AXIS_THRESHOLD)  && (DIPad_->PrevY() >= -AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->Y() >= -AXIS_THRESHOLD)  && (DIPad_->PrevY() <  -AXIS_THRESHOLD); });
		}
		else
		{
			aDele.Add([this](){ return  DIPad_->Y() >  AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->Y() >  AXIS_THRESHOLD)  && (DIPad_->PrevY() <= AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->Y() <= AXIS_THRESHOLD)  && (DIPad_->PrevY() >  AXIS_THRESHOLD); });
		}
	break;

	case DIGamePad_AXIS_Z:
		if(bNegative)
		{
			aDele.Add([this](){ return  DIPad_->Z() <  -AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->Z() <  -AXIS_THRESHOLD)  && (DIPad_->PrevZ() >= -AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->Z() >= -AXIS_THRESHOLD)  && (DIPad_->PrevZ() <  -AXIS_THRESHOLD); });
		}
		else
		{
			aDele.Add([this](){ return  DIPad_->Z() >  AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->Z() >  AXIS_THRESHOLD)  && (DIPad_->PrevZ() <= AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->Z() <= AXIS_THRESHOLD)  && (DIPad_->PrevZ() >  AXIS_THRESHOLD); });
		}
	break;

	case DIGamePad_ROT_X:
		if(bNegative)
		{
			aDele.Add([this](){ return  DIPad_->RotX() <  -AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->RotX() <  -AXIS_THRESHOLD)  && (DIPad_->RotPrevX() >= -AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->RotX() >= -AXIS_THRESHOLD)  && (DIPad_->RotPrevX() <  -AXIS_THRESHOLD); });
		}
		else
		{
			aDele.Add([this](){ return  DIPad_->RotX() >  AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->RotX() >  AXIS_THRESHOLD)  && (DIPad_->RotPrevX() <= AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->RotX() <= AXIS_THRESHOLD)  && (DIPad_->RotPrevX() >  AXIS_THRESHOLD); });
		}
	break;

	case DIGamePad_ROT_Y:
		if(bNegative)
		{
			aDele.Add([this](){ return  DIPad_->RotY() <  -AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->RotY() <  -AXIS_THRESHOLD)  && (DIPad_->RotPrevY() >= -AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->RotY() >= -AXIS_THRESHOLD)  && (DIPad_->RotPrevY() <  -AXIS_THRESHOLD); });
		}
		else
		{
			aDele.Add([this](){ return  DIPad_->RotY() >  AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->RotY() >  AXIS_THRESHOLD)  && (DIPad_->RotPrevY() <= AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->RotY() <= AXIS_THRESHOLD)  && (DIPad_->RotPrevY() >  AXIS_THRESHOLD); });
		}
	break;

	case DIGamePad_ROT_Z:
		if(bNegative)
		{
			aDele.Add([this](){ return  DIPad_->RotZ() <  -AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->RotZ() <  -AXIS_THRESHOLD)  && (DIPad_->RotPrevZ() >= -AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->RotZ() >= -AXIS_THRESHOLD)  && (DIPad_->RotPrevZ() <  -AXIS_THRESHOLD); });
		}
		else
		{
			aDele.Add([this](){ return  DIPad_->RotZ() >  AXIS_THRESHOLD; });
			aDele.Add([this](){ return (DIPad_->RotZ() >  AXIS_THRESHOLD)  && (DIPad_->RotPrevZ() <= AXIS_THRESHOLD); });
			aDele.Add([this](){ return (DIPad_->RotZ() <= AXIS_THRESHOLD)  && (DIPad_->RotPrevZ() >  AXIS_THRESHOLD); });
		}
	break;

	case DIGamePad_END:
		aDele.Add(nullptr);
		aDele.Add(nullptr);
		aDele.Add(nullptr);
	break;

	default:
		aDele.Add([this,eDIKey](){ return DIPad_->IsPress(eDIKey-DIGamePad_Button1); });
		aDele.Add([this,eDIKey](){ return DIPad_->IsPush(eDIKey-DIGamePad_Button1); });
		aDele.Add([this,eDIKey](){ return DIPad_->IsRelease(eDIKey-DIGamePad_Button1); });
	break;
	}

	return std::move(aDele);
}

void FXInputJoystickEmu::Event(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
	if(DIPad_->Input())
	{
		DIPad_->Event(MessageHandler);

		EventAnalog(FGamepadKeyNames::LeftAnalogX,			LeftAnalogX_,		MessageHandler);
		EventAnalog(FGamepadKeyNames::LeftAnalogY,			LeftAnalogY_,		MessageHandler);
		EventAnalog(FGamepadKeyNames::RightAnalogX,			RightAnalogX_,		MessageHandler);
		EventAnalog(FGamepadKeyNames::RightAnalogY,			RightAnalogY_,		MessageHandler);
		EventAnalog(FGamepadKeyNames::LeftTriggerAnalog,	LeftTriggerAnalog_,	MessageHandler);
		EventAnalog(FGamepadKeyNames::RightTriggerAnalog,	RightTriggerAnalog_,MessageHandler);

		EventButton(FGamepadKeyNames::LeftStickUp,		LeftStickUp_,	MessageHandler);
		EventButton(FGamepadKeyNames::LeftStickDown,	LeftStickDown_, MessageHandler);
		EventButton(FGamepadKeyNames::LeftStickRight,	LeftStickRight_,MessageHandler);
		EventButton(FGamepadKeyNames::LeftStickLeft,	LeftStickLeft_, MessageHandler);

		EventButton(FGamepadKeyNames::RightStickUp,		RightStickUp_,	 MessageHandler);
		EventButton(FGamepadKeyNames::RightStickDown,	RightStickDown_, MessageHandler);
		EventButton(FGamepadKeyNames::RightStickRight,	RightStickRight_,MessageHandler);
		EventButton(FGamepadKeyNames::RightStickLeft,	RightStickLeft_, MessageHandler);

		EventButton(FGamepadKeyNames::FaceButtonBottom, BtnA_, MessageHandler);
		EventButton(FGamepadKeyNames::FaceButtonRight,	BtnB_, MessageHandler);
		EventButton(FGamepadKeyNames::FaceButtonLeft,	BtnX_, MessageHandler);
		EventButton(FGamepadKeyNames::FaceButtonTop,	BtnY_, MessageHandler);

		EventButton(FGamepadKeyNames::LeftShoulder,  BtnLB_, MessageHandler);
		EventButton(FGamepadKeyNames::RightShoulder, BtnRB_, MessageHandler);

		EventButton(FGamepadKeyNames::LeftTriggerThreshold,	LeftTrigger_, MessageHandler);
		EventButton(FGamepadKeyNames::RightTriggerThreshold,RightTrigger_, MessageHandler);

		EventButton(FGamepadKeyNames::SpecialLeft,  BtnBack_, MessageHandler);
		EventButton(FGamepadKeyNames::SpecialRight, BtnStart_, MessageHandler);

		EventButton(FGamepadKeyNames::LeftThumb,	BtnLStick_, MessageHandler);
		EventButton(FGamepadKeyNames::RightThumb,	BtnRStick_, MessageHandler);

		EventButton(FGamepadKeyNames::DPadUp,	 DPadUp_, MessageHandler);
		EventButton(FGamepadKeyNames::DPadDown,  DPadDown_, MessageHandler);
		EventButton(FGamepadKeyNames::DPadRight, DPadRight_, MessageHandler);
		EventButton(FGamepadKeyNames::DPadLeft,  DPadLeft_, MessageHandler);
	}
}

void FXInputJoystickEmu::EventAnalog(FName XIName, const AnalogDelegate& Delegate, const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
	float Analog = 0.0f;

	if(Delegate.Axis_)
	{
		Analog = Delegate.Axis_();
		if(FMath::IsNearlyEqual(Analog, 0.0f) && Delegate.AxisNegative_)
		{	Analog = Delegate.AxisNegative_();	}

		if(XIName==FGamepadKeyNames::LeftTriggerAnalog || XIName==FGamepadKeyNames::RightTriggerAnalog)
		{
			Analog = FMath::GetMappedRangeValueUnclamped(FVector2D(-1.f,1.f), FVector2D(0.f,1.f), Analog);
		}
	}

	MessageHandler->OnControllerAnalog(XIName, DIPad_->GetPlayerIndex(), Analog);
}

void FXInputJoystickEmu::EventButton(FName XIName, const BtnDelegate& Delegate, const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
	if(Delegate.IsPush_ && Delegate.IsPush_())
		MessageHandler->OnControllerButtonPressed(XIName, DIPad_->GetPlayerIndex(), false);
	else if(Delegate.IsRelease_ && Delegate.IsRelease_())
		MessageHandler->OnControllerButtonReleased(XIName, DIPad_->GetPlayerIndex(), false);
}


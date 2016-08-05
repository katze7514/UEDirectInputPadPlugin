#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputJoystick.h"
#include "XInputJoystickEmu.h"

namespace{
const float AXIS_THRESHOLD = 0.6f;
}

void XInputJoystickEmu::Init(TSharedRef<FDirectInputJoystick> DIPad)
{
	DIPad_ = DIPad;

	SetAnalogMap(XIGamePad_LeftAnalogX,		DIGamePad_AXIS_X);
	SetAnalogMap(XIGamePad_LeftAnalogY,		DIGamePad_AXIS_Y);
	SetAnalogMap(XIGamePad_RightAnalogX,	DIGamePad_AXIS_Z);
	SetAnalogMap(XIGamePad_RightAnalogY,	DIGamePad_ROT_Z);
	SetAnalogMap(XIGamePad_LeftTrigger,		DIGamePad_Button7);
	SetAnalogMap(XIGamePad_RightTrigger,	DIGamePad_Button8);

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

void XInputJoystickEmu::SetAnalogMap(EXInputPadKeyNames eXIKey, EDirectInputPadKeyNames eDIKey, bool bNegative)
{
	if(eDIKey>=DIGamePad_ROT_Z)
	{
		switch(eXIKey)
		{
		case XIGamePad_LeftAnalogX:
			SetAnalogMapAnalog(eDIKey, LeftAnalogX_, LeftStickUp_, LeftStickDown_);		
		break;

		case XIGamePad_LeftAnalogY:
			SetAnalogMapAnalog(eDIKey, LeftAnalogY_, LeftStickRight_, LeftStickLeft_);
		break;

		case XIGamePad_RightAnalogX:
			SetAnalogMapAnalog(eDIKey, RightAnalogX_, RightStickUp_, RightStickDown_);
		break;

		case XIGamePad_RightAnalogY:
			SetAnalogMapAnalog(eDIKey, RightAnalogY_, RightStickRight_, RightStickLeft_);
		break;

		case XIGamePad_LeftTrigger:
		{
			BtnDelegate b;
			SetAnalogMapAnalog(eDIKey, LeftTriggerAnalog_, LeftTrigger_, b);
		}
		break;

		case XIGamePad_RightTrigger:
		{
			BtnDelegate b;
			SetAnalogMapAnalog(eDIKey, RightTriggerAnalog_, RightTrigger_, b);
		}
		break;
		}
	}
	else
	{
		switch(eXIKey)
		{
		case XIGamePad_LeftAnalogX:
			SetAnalogMapBtn(eDIKey, LeftAnalogX_, bNegative ? LeftStickDown_ : LeftStickUp_, bNegative);
		break;

		case XIGamePad_LeftAnalogY:
			SetAnalogMapBtn(eDIKey, LeftAnalogY_, bNegative ? LeftStickLeft_ : LeftStickRight_, bNegative);
		break;

		case XIGamePad_RightAnalogX:
			SetAnalogMapBtn(eDIKey, RightAnalogX_, bNegative ? RightStickDown_ : RightStickUp_, bNegative);
		break;

		case XIGamePad_RightAnalogY:
			SetAnalogMapBtn(eDIKey, RightAnalogY_, bNegative ? RightStickLeft_ : RightStickRight_, bNegative);
		break;

		case XIGamePad_LeftTrigger:
			SetAnalogMapBtn(eDIKey, LeftTriggerAnalog_, LeftTrigger_, false);
		break;

		case XIGamePad_RightTrigger:
			SetAnalogMapBtn(eDIKey, RightTriggerAnalog_, RightTrigger_, false);
		break;
		}
	}
}

namespace{
inline void SetBtnDelegate(BtnDelegate& Btn, const TArray<std::function<bool()>>& aBtnDele)
{
	Btn.IsPress_	= aBtnDele[0];
	Btn.IsPush_		= aBtnDele[1];
	Btn.IsRelease_	= aBtnDele[2];
}
}

void XInputJoystickEmu::SetAnalogMapAnalog(EDirectInputPadKeyNames eDIKey, AnalogDelegate& Analog, BtnDelegate& Pos, BtnDelegate& Neg)
{
	Analog.Axis_			= CreateAnalogDelegate(eDIKey);
	Analog.AxisNegative_	= nullptr;

	SetBtnDelegate(Pos, CreateButtonDelegate(eDIKey, false));
	SetBtnDelegate(Neg, CreateButtonDelegate(eDIKey, true));
}

void XInputJoystickEmu::SetAnalogMapBtn(EDirectInputPadKeyNames eDIKey, AnalogDelegate& Analog, BtnDelegate& Btn, bool bNegative)
{
	if(bNegative)
		Analog.AxisNegative_ = CreateAnalogDelegate(eDIKey,true);
	else
		Analog.Axis_ = CreateAnalogDelegate(eDIKey,false);

	SetBtnDelegate(Btn, CreateButtonDelegate(eDIKey));
}

void XInputJoystickEmu::SetButtonMap(EXInputPadKeyNames eXIKey, EDirectInputPadKeyNames eDIKey, bool bNegative)
{
	TArray<std::function<bool()>> aBtnDele;

	switch(eXIKey)
	{
	case XIGamePad_Button_A:
		SetBtnDelegate(BtnA_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_B:
		SetBtnDelegate(BtnB_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_X:
		SetBtnDelegate(BtnX_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_Y:
		SetBtnDelegate(BtnY_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_LB:
		SetBtnDelegate(BtnLB_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_RB:
		SetBtnDelegate(BtnRB_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_BACK:
		SetBtnDelegate(BtnBack_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_START:
		SetBtnDelegate(BtnStart_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_LStick:
		SetBtnDelegate(BtnLStick_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_Button_RStick:
		SetBtnDelegate(BtnRStick_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_DPad_Up:
		SetBtnDelegate(DPadUp_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_DPad_Down:
		SetBtnDelegate(DPadDown_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_DPad_Right:
		SetBtnDelegate(DPadRight_, CreateButtonDelegate(eDIKey, bNegative));
	break;

	case XIGamePad_DPad_Left:
		SetBtnDelegate(DPadLeft_, CreateButtonDelegate(eDIKey, bNegative));
	break;
	}
}

std::function<float()> XInputJoystickEmu::CreateAnalogDelegate(EDirectInputPadKeyNames eDIKey, bool bNegative)
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

	default:
		if(bNegative)
			return [this,eDIKey](){ return DIPad_->IsPress(eDIKey-DIGamePad_Button1) ? -1.0f : 0.0f; };
		else
			return [this,eDIKey](){ return DIPad_->IsPress(eDIKey-DIGamePad_Button1) ? 1.0f : 0.0f; };
	break;
	}

	return nullptr;
}

TArray<std::function<bool()>> XInputJoystickEmu::CreateButtonDelegate(EDirectInputPadKeyNames eDIKey, bool bNegative)
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

	default:
		aDele.Add([this,eDIKey](){ return DIPad_->IsPress(eDIKey-DIGamePad_Button1); });
		aDele.Add([this,eDIKey](){ return DIPad_->IsPush(eDIKey-DIGamePad_Button1); });
		aDele.Add([this,eDIKey](){ return DIPad_->IsRelease(eDIKey-DIGamePad_Button1); });
	break;
	}

	return std::move(aDele);
}

void XInputJoystickEmu::Event(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
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

void XInputJoystickEmu::EventAnalog(FName XIName, const AnalogDelegate& Delegate, const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
	float Analog = Delegate.Axis_();
	if(FMath::IsNearlyEqual(Analog, 0.0f) && Delegate.AxisNegative_)
	{	Analog = Delegate.AxisNegative_();	}

	if(Delegate.bReverse_)
	{	Analog = -Analog; }

	MessageHandler->OnControllerAnalog(XIName, DIPad_->GetPlayerIndex(), Analog);
}

void XInputJoystickEmu::EventButton(FName XIName, const BtnDelegate& Delegate, const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler)
{
	if(Delegate.IsPush_())
		MessageHandler->OnControllerButtonPressed(XIName, DIPad_->GetPlayerIndex(), false);
	else if(Delegate.IsRelease_())
		MessageHandler->OnControllerButtonReleased(XIName, DIPad_->GetPlayerIndex(), false);
}


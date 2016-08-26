#pragma once

#include "DirectInputPadState.generated.h"

//* Joystickの実キーを指すための列挙型
UENUM(BlueprintType)
enum EDirectInputPadKeyNames
{
	DIGamePad_AXIS_X UMETA(DisplayName = "DIGamePad Axis X"),
	DIGamePad_AXIS_Y UMETA(DisplayName = "DIGamePad Axis Y"),
	DIGamePad_AXIS_Z UMETA(DisplayName = "DIGamePad Axis Z"),
	DIGamePad_ROT_X UMETA(DisplayName = "DIGamePad Rot X"),
	DIGamePad_ROT_Y UMETA(DisplayName = "DIGamePad Rot Y"),
	DIGamePad_ROT_Z UMETA(DisplayName = "DIGamePad Rot Z"),

	DIGamePad_Button1 UMETA(DisplayName = "DIGamePad Button 1"),
	DIGamePad_Button2 UMETA(DisplayName = "DIGamePad Button 2"),
	DIGamePad_Button3 UMETA(DisplayName = "DIGamePad Button 3"),
	DIGamePad_Button4 UMETA(DisplayName = "DIGamePad Button 4"),
	DIGamePad_Button5 UMETA(DisplayName = "DIGamePad Button 5"),
	DIGamePad_Button6 UMETA(DisplayName = "DIGamePad Button 6"),
	DIGamePad_Button7 UMETA(DisplayName = "DIGamePad Button 7"),
	DIGamePad_Button8 UMETA(DisplayName = "DIGamePad Button 8"),
	DIGamePad_Button9 UMETA(DisplayName = "DIGamePad Button 9"),
	DIGamePad_Button10 UMETA(DisplayName = "DIGamePad Button 10"),
	DIGamePad_Button11 UMETA(DisplayName = "DIGamePad Button 11"),
	DIGamePad_Button12 UMETA(DisplayName = "DIGamePad Button 12"),

	DIGamePad_Button13 UMETA(DisplayName = "DIGamePad Button 13"),
	DIGamePad_Button14 UMETA(DisplayName = "DIGamePad Button 14"),
	DIGamePad_Button15 UMETA(DisplayName = "DIGamePad Button 15"),
	DIGamePad_Button16 UMETA(DisplayName = "DIGamePad Button 16"),
	DIGamePad_Button17 UMETA(DisplayName = "DIGamePad Button 17"),
	DIGamePad_Button18 UMETA(DisplayName = "DIGamePad Button 18"),
	DIGamePad_Button19 UMETA(DisplayName = "DIGamePad Button 19"),
	DIGamePad_Button20 UMETA(DisplayName = "DIGamePad Button 20"),
	DIGamePad_Button21 UMETA(DisplayName = "DIGamePad Button 21"),
	DIGamePad_Button22 UMETA(DisplayName = "DIGamePad Button 22"),
	DIGamePad_Button23 UMETA(DisplayName = "DIGamePad Button 23"),
	DIGamePad_Button24 UMETA(DisplayName = "DIGamePad Button 24"),
	DIGamePad_Button25 UMETA(DisplayName = "DIGamePad Button 25"),
	DIGamePad_Button26 UMETA(DisplayName = "DIGamePad Button 26"),
	DIGamePad_Button27 UMETA(DisplayName = "DIGamePad Button 27"),
	DIGamePad_Button28 UMETA(DisplayName = "DIGamePad Button 28"),
	DIGamePad_Button29 UMETA(DisplayName = "DIGamePad Button 29"),
	DIGamePad_Button30 UMETA(DisplayName = "DIGamePad Button 30"),
	DIGamePad_Button31 UMETA(DisplayName = "DIGamePad Button 31"),
	DIGamePad_Button32 UMETA(DisplayName = "DIGamePad Button 32"),

	DIGamePad_POV_Up	UMETA(DisplayName = "DIGamePad POV Up"),
	DIGamePad_POV_Right	UMETA(DisplayName = "DIGamePad POV Down"),
	DIGamePad_POV_Down	UMETA(DisplayName = "DIGamePad POV Right"),
	DIGamePad_POV_Left	UMETA(DisplayName = "DIGamePad POV Left"),

	DIGamePad_END UMETA(DisplayName = "DIGamePad END"),
};

/* XInputのボタン配置を表す列挙型
/* XBOXパッドの名称に合わせてある */
UENUM(BlueprintType)
enum EXInputPadKeyNames
{
	XIGamePad_LeftAnalogX		UMETA(DisplayName = "XIGamePad Left Analog X"),
	XIGamePad_LeftAnalogY		UMETA(DisplayName = "XIGamePad Left Analog Y"),
	XIGamePad_RightAnalogX		UMETA(DisplayName = "XIGamePad Right Analog X"),
	XIGamePad_RightAnalogY		UMETA(DisplayName = "XIGamePad Right Analog Y"),
	XIGamePad_LTAnalog			UMETA(DisplayName = "XIGamePad LT Analog"),
	XIGamePad_RTAnalog			UMETA(DisplayName = "XIGamePad RT Analog"),

	XIGamePad_Button_A			UMETA(DisplayName = "XIGamePad Button A"),
	XIGamePad_Button_B			UMETA(DisplayName = "XIGamePad Button B"),
	XIGamePad_Button_X			UMETA(DisplayName = "XIGamePad Button X"),
	XIGamePad_Button_Y			UMETA(DisplayName = "XIGamePad Button Y"),

	XIGamePad_Button_LB			UMETA(DisplayName = "XIGamePad Button LB"),
	XIGamePad_Button_RB			UMETA(DisplayName = "XIGamePad Button RB"),
	XIGamePad_Button_LT			UMETA(DisplayName = "XIGamePad Button LT"),
	XIGamePad_Button_RT			UMETA(DisplayName = "XIGamePad Button RT"),

	XIGamePad_Button_BACK		UMETA(DisplayName = "XIGamePad Button BACK"),
	XIGamePad_Button_START		UMETA(DisplayName = "XIGamePad Button START"),

	XIGamePad_Button_LStick		UMETA(DisplayName = "XIGamePad Button L Stick"),
	XIGamePad_Button_RStick		UMETA(DisplayName = "XIGamePad Button R Stick"),

	XIGamePad_DPad_Up			UMETA(DisplayName = "XIGamePad DPad Up"),
	XIGamePad_DPad_Down			UMETA(DisplayName = "XIGamePad DPad Down"),
	XIGamePad_DPad_Right		UMETA(DisplayName = "XIGamePad DPad Right"),
	XIGamePad_DPad_Left			UMETA(DisplayName = "XIGamePad DPad Left"),

	XIGamePad_END UMETA(DisplayName = "XIGamePad END")
};

// DirectInputPad用FKey
struct EKeysDirectInputPad
{
	static const FKey DIGamePad_AxisX;
	static const FKey DIGamePad_AxisY;
	static const FKey DIGamePad_AxisZ;
	static const FKey DIGamePad_RotX;
	static const FKey DIGamePad_RotY;
	static const FKey DIGamePad_RotZ;

	//static const FKey DIGamePad_POV;
	static const FKey DIGamePad_POV_Up;
	static const FKey DIGamePad_POV_Left;
	static const FKey DIGamePad_POV_Down;
	static const FKey DIGamePad_POV_Right;

	static const FKey DIGamePad_Button1;
	static const FKey DIGamePad_Button2;
	static const FKey DIGamePad_Button3;
	static const FKey DIGamePad_Button4;
	static const FKey DIGamePad_Button5;
	static const FKey DIGamePad_Button6;
	static const FKey DIGamePad_Button7;
	static const FKey DIGamePad_Button8;
	static const FKey DIGamePad_Button9;
	static const FKey DIGamePad_Button10;
	static const FKey DIGamePad_Button11;
	static const FKey DIGamePad_Button12;

	static const FKey DIGamePad_Button13;
	static const FKey DIGamePad_Button14;
	static const FKey DIGamePad_Button15;
	static const FKey DIGamePad_Button16;
	static const FKey DIGamePad_Button17;
	static const FKey DIGamePad_Button18;
	static const FKey DIGamePad_Button19;
	static const FKey DIGamePad_Button20;
	static const FKey DIGamePad_Button21;
	static const FKey DIGamePad_Button22;
	static const FKey DIGamePad_Button23;
	static const FKey DIGamePad_Button24;
	static const FKey DIGamePad_Button25;
	static const FKey DIGamePad_Button26;
	static const FKey DIGamePad_Button27;
	static const FKey DIGamePad_Button28;
	static const FKey DIGamePad_Button29;
	static const FKey DIGamePad_Button30;
	static const FKey DIGamePad_Button31;
	static const FKey DIGamePad_Button32;
};

USTRUCT(BlueprintType)
struct DIRECTINPUTPADPLUGIN_API FDIKeyMapInfo
{
	GENERATED_BODY()

	FDIKeyMapInfo(EDirectInputPadKeyNames eDIKey=DIGamePad_END, bool bNegative=false)
		: eDIKey_(eDIKey), bNegative_(bNegative){}


	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDirectInputPadKeyNames> eDIKey_ = DIGamePad_END;

	UPROPERTY(BlueprintReadOnly)
	bool bNegative_ = false;
};

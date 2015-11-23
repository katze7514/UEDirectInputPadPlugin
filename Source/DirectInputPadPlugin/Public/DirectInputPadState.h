#pragma once

//* Joystickの実キーを指すための列挙型
UENUM(BluprintableType)
enum EDirectInputPadKeyName : uint8
{
	DIGamePad_AXIS_X UMETA(DisplayName = "DIGamePad_AxisX"),
	DIGamePad_AXIS_Y UMETA(DisplayName = "DIGamePad_AxisX"),
	DIGamePad_AXIS_Z UMETA(DisplayName = "DIGamePad_AxisX"),
	DIGamePad_ROT_X UMETA(DisplayName = "DIGamePad_RotX"),
	DIGamePad_ROT_Y UMETA(DisplayName = "DIGamePad_RotX"),
	DIGamePad_ROT_Z UMETA(DisplayName = "DIGamePad_RotX"),

	DIGamePad_POV UMETA(DisplayName = "DIGamePad_POV"),

	DIGamePad_Button1 UMETA(DisplayName = "DIGamePad_Button1"),
	DIGamePad_Button2 UMETA(DisplayName = "DIGamePad_Button2"),
	DIGamePad_Button3 UMETA(DisplayName = "DIGamePad_Button3"),
	DIGamePad_Button4 UMETA(DisplayName = "DIGamePad_Button4"),
	DIGamePad_Button5 UMETA(DisplayName = "DIGamePad_Button5"),
	DIGamePad_Button6 UMETA(DisplayName = "DIGamePad_Button6"),
	DIGamePad_Button7 UMETA(DisplayName = "DIGamePad_Button7"),
	DIGamePad_Button8 UMETA(DisplayName = "DIGamePad_Button8"),
	DIGamePad_Button9 UMETA(DisplayName = "DIGamePad_Button9"),
	DIGamePad_Button10 UMETA(DisplayName = "DIGamePad_Button10"),
	DIGamePad_Button11 UMETA(DisplayName = "DIGamePad_Button11"),
	DIGamePad_Button12 UMETA(DisplayName = "DIGamePad_Button12"),

	DIGamePad_Button13 UMETA(DisplayName = "DIGamePad_Button13"),
	DIGamePad_Button14 UMETA(DisplayName = "DIGamePad_Button14"),
	DIGamePad_Button15 UMETA(DisplayName = "DIGamePad_Button15"),
	DIGamePad_Button16 UMETA(DisplayName = "DIGamePad_Button16"),
	DIGamePad_Button17 UMETA(DisplayName = "DIGamePad_Button17"),
	DIGamePad_Button18 UMETA(DisplayName = "DIGamePad_Button18"),
	DIGamePad_Button19 UMETA(DisplayName = "DIGamePad_Button19"),
	DIGamePad_Button20 UMETA(DisplayName = "DIGamePad_Button20"),
	DIGamePad_Button21 UMETA(DisplayName = "DIGamePad_Button21"),
	DIGamePad_Button22 UMETA(DisplayName = "DIGamePad_Button22"),
	DIGamePad_Button23 UMETA(DisplayName = "DIGamePad_Button23"),
	DIGamePad_Button24 UMETA(DisplayName = "DIGamePad_Button24"),
	DIGamePad_Button25 UMETA(DisplayName = "DIGamePad_Button25"),
	DIGamePad_Button26 UMETA(DisplayName = "DIGamePad_Button26"),
	DIGamePad_Button27 UMETA(DisplayName = "DIGamePad_Button27"),
	DIGamePad_Button28 UMETA(DisplayName = "DIGamePad_Button28"),
	DIGamePad_Button29 UMETA(DisplayName = "DIGamePad_Button29"),
	DIGamePad_Button30 UMETA(DisplayName = "DIGamePad_Button30"),
	DIGamePad_Button31 UMETA(DisplayName = "DIGamePad_Button31"),
	DIGamePad_Button32 UMETA(DisplayName = "DIGamePad_Button32"),

	DIGamePad_END UMETA(DisplayName = "DIGamePad_END"),
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

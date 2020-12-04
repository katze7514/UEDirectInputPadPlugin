// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "DirectInputPadPlugin.h"
#include "DirectInputPadPluginPrivatePCH.h"

#include "DirectInputPadState.h"
#include "DirectInputPadDevice.h"

IMPLEMENT_MODULE( FDirectInputPadPlugin, DirectInputPadPlugin )

DEFINE_LOG_CATEGORY(LogDirectInputPadPlugin);

#define LOCTEXT_NAMESPACE "DirectInputPadPlugin"

namespace{
TSharedPtr<FDirectInputPadDevice> device_;
}

void FDirectInputPadPlugin::StartupModule()
{
	IDirectInputPadPlugin::StartupModule();
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

#define ADDKEY(keyname,text0,text1,attr) EKeys::AddKey(FKeyDetails(EKeysDirectInputPad::keyname, LOCTEXT(text0, text1), attr))
#define ATTR_ANALOG (FKeyDetails::GamepadKey|FKeyDetails::FloatAxis)
#define ATTR_BTN (FKeyDetails::GamepadKey)

	ADDKEY(DIGamePad_AxisX, "DIGamePad_AxisX", "DIGamePad Axis X", ATTR_ANALOG);
	ADDKEY(DIGamePad_AxisY, "DIGamePad_AxisY", "DIGamePad Axis Y", ATTR_ANALOG);
	ADDKEY(DIGamePad_AxisZ, "DIGamePad_AxisZ", "DIGamePad Axis Z", ATTR_ANALOG);
	ADDKEY(DIGamePad_RotX,  "DIGamePad_RotX",  "DIGamePad Rot X",  ATTR_ANALOG);
	ADDKEY(DIGamePad_RotY,  "DIGamePad_RotY",  "DIGamePad Rot Y",  ATTR_ANALOG);
	ADDKEY(DIGamePad_RotZ,  "DIGamePad_RotZ",  "DIGamePad Rot Z",  ATTR_ANALOG);

	//ADDKEY(DIGamePad_POV,   "DIGamePad_POV",   "DIGamePad POV",    ATTR_ANALOG);
	ADDKEY(DIGamePad_POV_Up,	"DIGamePad_POV_Up",		"DIGamePad POV Up",		ATTR_BTN);
	ADDKEY(DIGamePad_POV_Left,	"DIGamePad_POV_Left",	"DIGamePad POV Left",	ATTR_BTN);
	ADDKEY(DIGamePad_POV_Down,	"DIGamePad_POV_Down",	"DIGamePad POV Down",	ATTR_BTN);
	ADDKEY(DIGamePad_POV_Right,	"DIGamePad_POV_Right",	"DIGamePad POV Right",	ATTR_BTN);

	ADDKEY(DIGamePad_Button1,  "DIGamePad_Button1",  "DIGamePad Button 1",  ATTR_BTN);
	ADDKEY(DIGamePad_Button2,  "DIGamePad_Button2",  "DIGamePad Button 2",  ATTR_BTN);
	ADDKEY(DIGamePad_Button3,  "DIGamePad_Button3",  "DIGamePad Button 3",  ATTR_BTN);
	ADDKEY(DIGamePad_Button4,  "DIGamePad_Button4",  "DIGamePad Button 4",  ATTR_BTN);
	ADDKEY(DIGamePad_Button5,  "DIGamePad_Button5",  "DIGamePad Button 5",  ATTR_BTN);
	ADDKEY(DIGamePad_Button6,  "DIGamePad_Button6",  "DIGamePad Button 6",  ATTR_BTN);
	ADDKEY(DIGamePad_Button7,  "DIGamePad_Button7",  "DIGamePad Button 7",  ATTR_BTN);
	ADDKEY(DIGamePad_Button8,  "DIGamePad_Button8",  "DIGamePad Button 8",  ATTR_BTN);
	ADDKEY(DIGamePad_Button9,  "DIGamePad_Button9",  "DIGamePad Button 9",  ATTR_BTN);
	ADDKEY(DIGamePad_Button10, "DIGamePad_Button10", "DIGamePad Button 10", ATTR_BTN);
	ADDKEY(DIGamePad_Button11, "DIGamePad_Button11", "DIGamePad Button 11", ATTR_BTN);
	ADDKEY(DIGamePad_Button12, "DIGamePad_Button12", "DIGamePad Button 12", ATTR_BTN);

	ADDKEY(DIGamePad_Button13, "DIGamePad_Button13", "DIGamePad Button 13", ATTR_BTN);
	ADDKEY(DIGamePad_Button14, "DIGamePad_Button14", "DIGamePad Button 14", ATTR_BTN);
	ADDKEY(DIGamePad_Button15, "DIGamePad_Button15", "DIGamePad Button 15", ATTR_BTN);
	ADDKEY(DIGamePad_Button16, "DIGamePad_Button16", "DIGamePad Button 16", ATTR_BTN);
	ADDKEY(DIGamePad_Button17, "DIGamePad_Button17", "DIGamePad Button 17", ATTR_BTN);
	ADDKEY(DIGamePad_Button18, "DIGamePad_Button18", "DIGamePad Button 18", ATTR_BTN);
	ADDKEY(DIGamePad_Button19, "DIGamePad_Button19", "DIGamePad Button 19", ATTR_BTN);
	ADDKEY(DIGamePad_Button20, "DIGamePad_Button20", "DIGamePad Button 20", ATTR_BTN);
	ADDKEY(DIGamePad_Button21, "DIGamePad_Button21", "DIGamePad Button 21", ATTR_BTN);
	ADDKEY(DIGamePad_Button22, "DIGamePad_Button22", "DIGamePad Button 22", ATTR_BTN);
	ADDKEY(DIGamePad_Button23, "DIGamePad_Button23", "DIGamePad Button 23", ATTR_BTN);
	ADDKEY(DIGamePad_Button24, "DIGamePad_Button24", "DIGamePad Button 24", ATTR_BTN);
	ADDKEY(DIGamePad_Button25, "DIGamePad_Button25", "DIGamePad Button 25", ATTR_BTN);
	ADDKEY(DIGamePad_Button26, "DIGamePad_Button26", "DIGamePad Button 26", ATTR_BTN);
	ADDKEY(DIGamePad_Button27, "DIGamePad_Button27", "DIGamePad Button 27", ATTR_BTN);
	ADDKEY(DIGamePad_Button28, "DIGamePad_Button28", "DIGamePad Button 28", ATTR_BTN);
	ADDKEY(DIGamePad_Button29, "DIGamePad_Button29", "DIGamePad Button 29", ATTR_BTN);
	ADDKEY(DIGamePad_Button30, "DIGamePad_Button30", "DIGamePad Button 30", ATTR_BTN);
	ADDKEY(DIGamePad_Button31, "DIGamePad_Button31", "DIGamePad Button 31", ATTR_BTN);
	ADDKEY(DIGamePad_Button32, "DIGamePad_Button32", "DIGamePad Button 32", ATTR_BTN);

#undef ATTR_ANALOG
#undef ATTR_BTN
#undef ADDKEY
}

void FDirectInputPadPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if(device_.IsValid()) device_->Fin();
}

TSharedPtr< class IInputDevice > FDirectInputPadPlugin::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	if(device_.IsValid()) return device_;
	
	bool bBackGround = false;

	check(GConfig);
	GConfig->GetBool(TEXT("DirectInputPadPlugin"), TEXT("Background"), bBackGround, GInputIni);

	//UE_LOG(LogDirectInputPadPlugin,Log, TEXT("Background %d"),bBackGround);

	device_ = MakeShareable<FDirectInputPadDevice>(new FDirectInputPadDevice());
	if(device_->Init(bBackGround))
	{
		device_->SetMessageHandler(InMessageHandler);
		return device_;
	}

	device_ = nullptr;
	return nullptr;
}

TSharedPtr<FDirectInputPadDevice> FDirectInputPadPlugin::GetDirectInputPadDevice()
{
	return device_;
}

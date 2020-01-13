// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.
#include "Core.h"
#include "CoreUObject.h"
#include "Engine.h"
#include "SlateBasics.h"
#include "SlateCore.h"
#include "InputCore.h"
#include "InputDevice.h"

#include "DirectInputPadPlugin.h"

#include "Windows/AllowWindowsPlatformTypes.h"

#define DIRECTINPUT_VERSION 0x0800 
#define _CRT_SECURE_NO_DEPRECATE

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <windows.h>

#pragma warning(push) 
#pragma warning(disable:6000 28251) 
#include <dinput.h> 
#pragma warning(pop) 

#include <dinputd.h>

#include "Windows/HideWindowsPlatformTypes.h"

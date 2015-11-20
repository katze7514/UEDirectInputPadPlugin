// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "DirectInputPadPluginPrivatePCH.h"
#include "DirectInputPadPlugin.h"

IMPLEMENT_MODULE( FDirectInputPadPlugin, DirectInputPadPlugin )

DEFINE_LOG_CATEGORY_STATIC(DirectInputPadPlugin, Log, All)


void FDirectInputPadPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FDirectInputPadPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

TSharedPtr< class IInputDevice > FDirectInputPadPlugin::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	return nullptr;
}



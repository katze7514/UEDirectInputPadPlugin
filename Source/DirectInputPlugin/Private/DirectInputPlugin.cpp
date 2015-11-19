// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "DirectInputPluginPrivatePCH.h"
#include "DirectInputPlugin.h"

IMPLEMENT_MODULE( FDirectInputPlugin, DirectInputPlugin )

DEFINE_LOG_CATEGORY_STATIC(DirectInputPlugin, Log, All)


void FDirectInputPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FDirectInputPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

TSharedPtr< class IInputDevice > FDirectInputPlugin::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	return nullptr;
}



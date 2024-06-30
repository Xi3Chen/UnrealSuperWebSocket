// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealSuperWebSocketEditSettingsCommands.h"

#define LOCTEXT_NAMESPACE "FUnrealSuperWebSocketEditSettingsModule"

void FUnrealSuperWebSocketEditSettingsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "UnrealSuperWebSocketEditSettings", "Execute UnrealSuperWebSocketEditSettings action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UnrealSuperWebSocketEditSettingsStyle.h"

class FUnrealSuperWebSocketEditSettingsCommands : public TCommands<FUnrealSuperWebSocketEditSettingsCommands>
{
public:

	FUnrealSuperWebSocketEditSettingsCommands()
		: TCommands<FUnrealSuperWebSocketEditSettingsCommands>(TEXT("UnrealSuperWebSocketEditSettings"), NSLOCTEXT("Contexts", "UnrealSuperWebSocketEditSettings", "UnrealSuperWebSocketEditSettings Plugin"), NAME_None, FUnrealSuperWebSocketEditSettingsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};

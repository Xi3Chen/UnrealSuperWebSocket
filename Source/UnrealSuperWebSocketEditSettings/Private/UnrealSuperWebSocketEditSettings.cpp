// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealSuperWebSocketEditSettings.h"
#include "UnrealSuperWebSocketEditSettingsStyle.h"
#include "UnrealSuperWebSocketEditSettingsCommands.h"
#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"
#include "ToolMenus.h"
#include "Interfaces/IPluginManager.h"

static const FName UnrealSuperWebSocketEditSettingsTabName("UnrealSuperWebSocketEditSettings");

#define LOCTEXT_NAMESPACE "FUnrealSuperWebSocketEditSettingsModule"

void FUnrealSuperWebSocketEditSettingsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUnrealSuperWebSocketEditSettingsStyle::Initialize();
	FUnrealSuperWebSocketEditSettingsStyle::ReloadTextures();

	FUnrealSuperWebSocketEditSettingsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUnrealSuperWebSocketEditSettingsCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FUnrealSuperWebSocketEditSettingsModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUnrealSuperWebSocketEditSettingsModule::RegisterMenus));
}

void FUnrealSuperWebSocketEditSettingsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUnrealSuperWebSocketEditSettingsStyle::Shutdown();

	FUnrealSuperWebSocketEditSettingsCommands::Unregister();
}

void FUnrealSuperWebSocketEditSettingsModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	//get plugin path
	FString PugingPath = IPluginManager::Get().FindPlugin("UnrealSuperWebSocket")->GetBaseDir();
	FString UnrealInstallPath;
	FString EMSDK_Version;
	if (FPaths::DirectoryExists(PugingPath))
	{

	}
	FString ConfigPath = FPaths::Combine(PugingPath, TEXT("Resources/LibraryConfig.conf"));
	ShowInfos:
	if (FPaths::FileExists(ConfigPath))
	{
		//open file
		FString FileContent;
		
		if (FFileHelper::LoadFileToString(FileContent, *ConfigPath))
		{
			//获取LibraryConfig.conf中的两行内容
			TArray<FString> Lines;
			FileContent.ParseIntoArrayLines(Lines, true);
			if (Lines.Num() >= 2)
			{
				//获取第一行中UnrealInstallPath 后的=符号侯后面的字符串，并剔除空格和双引号
				UnrealInstallPath = Lines[0].RightChop(Lines[0].Find(TEXT("=")) + 1).TrimStartAndEnd();
				UnrealInstallPath.RemoveFromStart(TEXT("\""));
				UnrealInstallPath.RemoveFromEnd(TEXT("\""));

				//获取第二行中EMSDK_Version 后的=符号侯后面的字符串，并剔除空格和双引号
				EMSDK_Version = Lines[1].RightChop(Lines[1].Find(TEXT("=")) + 1).TrimStartAndEnd();
				EMSDK_Version.RemoveFromStart(TEXT("\""));
				EMSDK_Version.RemoveFromEnd(TEXT("\""));

			}
		}
	}
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Config infos: \nEngineInstallPath:{0}\nEMSDK_Version:{1}"),
							FText::FromString(UnrealInstallPath),
							FText::FromString(EMSDK_Version)
					   );
	EAppReturnType::Type ReturnValue = FMessageDialog::Open(EAppMsgType::CancelRetryContinue, DialogText);
	
	switch (ReturnValue)
	{
	case EAppReturnType::No:
		break;
	case EAppReturnType::Yes:
		break;
	case EAppReturnType::YesAll:
		break;
	case EAppReturnType::NoAll:
		break;
	case EAppReturnType::Cancel:
		break;
	case EAppReturnType::Ok:
		break;
	case EAppReturnType::Retry:
	{
		FString Paths = ResetConfig();
		goto ShowInfos;
	}
		break;
	case EAppReturnType::Continue:
		break;
	default:
		break;
	}
}

void FUnrealSuperWebSocketEditSettingsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUnrealSuperWebSocketEditSettingsCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUnrealSuperWebSocketEditSettingsCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

FString FUnrealSuperWebSocketEditSettingsModule::ResetConfig()
{
	FString EngineInstallPath = FPaths::ConvertRelativePathToFull(FPaths::EngineDir());
	FString RootEMSDKPath = FPaths::Combine(EngineInstallPath, TEXT("Platforms/HTML5/Build/emsdk"));
	//移除EngineInstallPath中的Engine/字符
	EngineInstallPath = EngineInstallPath.LeftChop(8);
	//遍历RootEMSDKPath路径下的子文件夹，
	TArray<FString>ChildDir;
	IFileManager::Get().FindFilesRecursive(ChildDir, *RootEMSDKPath, TEXT("*"), false, true);
	FString EmsdkVersion;
	for (auto it : ChildDir)
	{
		if (it.Contains(TEXT("emsdk-")))
		{
			TArray<FString> Version;
			it.ParseIntoArray(Version,TEXT("emsdk-"));
			EmsdkVersion = TEXT("emsdk-") + Version[1];
			break;
		}
	}
	FFileHelper::SaveStringToFile(FString(TEXT("UnrealInstallPath = \"") + EngineInstallPath + TEXT("\"\n") + TEXT("EMSDK_Version=\"") + EmsdkVersion + TEXT("\"")), *GetConfigPath());
	return EmsdkVersion;
}

FString FUnrealSuperWebSocketEditSettingsModule::GetConfigPath()
{
	static FString ConfigPath;
	if (ConfigPath.Len() == 0)
	{
		FString PugingPath = IPluginManager::Get().FindPlugin("UnrealSuperWebSocket")->GetBaseDir();
		ConfigPath = FPaths::Combine(PugingPath, TEXT("Resources/LibraryConfig.conf"));

	}
	return ConfigPath;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealSuperWebSocketEditSettingsModule, UnrealSuperWebSocketEditSettings)
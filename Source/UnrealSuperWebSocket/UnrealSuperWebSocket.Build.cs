// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using System;

using System.Runtime.Remoting.Messaging;
using UnrealBuildTool;



public class UnrealSuperWebSocket : ModuleRules
{
	public UnrealSuperWebSocket(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		string ModulePath = ModuleDirectory;
        string PlginPath = Path.GetFullPath(Path.Combine(ModulePath, "../../"));
		string LibraryConfigPath = Path.Combine(PlginPath, "Resources/LibraryConfig.conf");
        string LibraryConfigText = "";
        string UnrealInstallPath = "";
        string EMSDK_Version = "";
        if (File.Exists(LibraryConfigPath))
		{
			System.Console.WriteLine("Reading LibraryConfig.conf at path:" + LibraryConfigPath);
            // read LibraryConfig.json file
            using (StreamReader file = File.OpenText(LibraryConfigPath))
            {
                LibraryConfigText = file.ReadToEnd();
				System.Console.WriteLine("LibraryConfig.conf content is :\n" + LibraryConfigText);
                //获取LibraryConfig.conf中的两行内容
                string[] lines = LibraryConfigText.Split('\n');
                //检查数组元素个数是否为2
				if(lines.Length == 2)
				{
                    //获取第一行中=符号侯后面的字符串，并剔除空格和双引号
                    UnrealInstallPath = lines[0].Split('=')[1].Trim().Replace("\"", "");
                    //获取第二行中=符号侯后面的字符串，并剔除空格和双引号
                    EMSDK_Version = lines[1].Split('=')[1].Trim().Replace("\"", "");
                }
            }

        }
        else
		{
			string ErrorMessage = "LibraryConfig.conf is not found at :" + LibraryConfigPath;

            System.Console.WriteLine(ErrorMessage+ "\nPlease Reset LibraryConfig.conf. Press Enter to continue...");
            System.Console.ReadLine();
            throw new FileNotFoundException(ErrorMessage);
        }
        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		if(Target.Platform == UnrealTargetPlatform.Win64)
		{
            PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "WebSockets"
				// ... add private dependencies that you statically link with here ...	
			}
            );
        }
		else if(Target.Platform == UnrealTargetPlatform.HTML5)
		{
            PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
            );

			string ErrorMessage;

            string WEBSocket_jsPath = Path.Combine(UnrealInstallPath + ("/Engine/Platforms/HTML5/Build/emsdk/" + EMSDK_Version + "/upstream/emscripten/src/library_websocket.js"));
			if (File.Exists(WEBSocket_jsPath))
			{
				PublicAdditionalLibraries.Add(WEBSocket_jsPath);
				ErrorMessage = "add " + WEBSocket_jsPath+ " successfull";
                System.Console.WriteLine(ErrorMessage);
            }
			else
			{
				//throw library_websocket.js is not exit.
				 ErrorMessage = "library_websocket.js is not found at :" + WEBSocket_jsPath;
				System.Console.WriteLine(ErrorMessage);
				throw new FileNotFoundException(ErrorMessage);


			}
		}
		
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

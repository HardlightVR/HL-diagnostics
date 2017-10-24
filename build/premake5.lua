-- We are using PCH, so you must disable PCH on the protobuf files, and enable generation on stdafx.cpp


workspace "Diagnostics"
	configurations {"Debug", "Release"}
	platforms {"Win32"}
	language "C++"
	
	


project "DiagnosticTool" 
	kind "ConsoleApp"
	
	targetdir "bin/%{cfg.buildcfg}/%{cfg.platform}"
	targetname "DiagnosticTool"
	
	-- dependencies

	json_include_dir = "C:/Users/NullSpace Team/Documents/NS_Unreal_SDK/src/Driver/Json"
	boost_incl_dir = "D:/Libraries/boost/boost_1_61_0"




	flags {
		"MultiProcessorCompile",
		"C++11"

	}
	--links {"System", "UnityEditor", "UnityEngine", "System.ServiceProcess"}

	files {
		"../src/*.cpp",
		"../src/*.h",
		"../src/*.hpp",
		path.join(json_include_dir, "jsoncpp.cpp")
		
	}

	


	boost_win32_dir = "D:/Libraries/boost/boost_1_61_0/stage/win32/lib"
	boost_win64_dir = "D:/Libraries/boost/boost_1_61_0/stage/x64/lib"

	platform_include_dir = "C:/Users/NullSpace Team/Documents/NS_Unreal_SDK/src/Driver/runtime_include"
	hardlight_include_dir = "C:/Users/NullSpace Team/Documents/Visual Studio 2015/Projects/NSLoader/src/Plugin/include"

	platform_dir_release = "C:/Users/NullSpace Team/Documents/NS_Unreal_SDK/build/bin/Release/Win32"
	platform_dir_debug = "C:/Users/NullSpace Team/Documents/NS_Unreal_SDK/build/bin/Debug/Win32"

	hardlight_dir_release = "C:/Users/NullSpace Team/Documents/Visual Studio 2015/Projects/NSLoader/build/bin/Release/Win32"
	hardlight_dir_debug = "C:/Users/NullSpace Team/Documents/Visual Studio 2015/Projects/NSLoader/build/bin/Debug/Win32"

	pchheader "stdafx.h"
	pchsource "../src/stdafx.cpp"

	includedirs {
		json_include_dir,
		boost_incl_dir,
		platform_include_dir,
		hardlight_include_dir
	}
	
	
	defines { "NOMINMAX", "BOOST_THREAD_USE_LIB"}
	
	filter {"files:**jsoncpp.cpp"}
		flags {'NoPCH'}


	-- input: libprotobuf
	filter {"platforms:Win32"}
		libdirs {
			boost_win32_dir,
		}	
		links {"HardlightPlatform.lib", "Hardlight.lib", "d3d11.lib", "d3dcompiler.lib"}
	


	filter {"platforms:Win32", "configurations:Debug"}
		
		postbuildcommands {
			"{COPY} %{platform_dir_debug}/HardlightPlatform.dll %{cfg.targetdir}",
					"{COPY} %{hardlight_dir_debug}/Hardlight.dll %{cfg.targetdir}",

		--	"{COPY} ../src/imgui.ini %{cfg.targetdir}"

		}
		libdirs {
			platform_dir_debug,
			hardlight_dir_debug
		}

	filter {"platforms:Win32", "configurations:Release"}
		
		defines {"NDEBUG"}
		optimize "On" 
		postbuildcommands {
			"{COPY} %{platform_dir_release}/HardlightPlatform.dll %{cfg.targetdir}",
							"{COPY} %{hardlight_dir_release}/Hardlight.dll %{cfg.targetdir}",

		
			--"{COPY} ../src/imgui.ini %{cfg.targetdir}"

		}
		libdirs {
			platform_dir_release,
			hardlight_dir_release
		}



	filter {"system:Windows"}
		defines {"_WINDOWS"}




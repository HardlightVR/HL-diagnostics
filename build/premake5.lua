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

	hardlight_driver_include_dir = "C:/Users/NullSpace Team/Documents/NS_Unreal_SDK/src/Driver"
	hardlight_driver_lib_dir = "C:/Users/NullSpace Team/Documents/NS_Unreal_SDK/build/bin/Release/Win32"

	includedirs {
	json_include_dir,
	boost_incl_dir,
	hardlight_driver_include_dir
	}

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


	nullspace_win32_dir = "C:/Users/NullSpace Team/Documents/Visual Studio 2015/Projects/NSLoader/build/bin"
	
	pchheader "stdafx.h"
	pchsource "../src/stdafx.cpp"

	
	defines { "NOMINMAX", "BOOST_THREAD_USE_LIB"}
	
	filter {"files:**jsoncpp.cpp"}
		flags {'NoPCH'}


	-- input: libprotobuf
	filter {"platforms:Win32"}
		libdirs {
			boost_win32_dir,
			hardlight_driver_lib_dir
		}	
		links {"HardlightPlatform.lib", "d3d11.lib", "d3dcompiler.lib"}
	


	filter {"platforms:Win32", "configurations:Debug"}
		libdirs {
			path.join(nullspace_win32_dir, "Debug/Win32")
		}

		postbuildcommands {
			"{COPY} ../../NSLoader/build/bin/Debug/Win32/Hardlight.dll %{cfg.targetdir}",
		
			"{COPY} ../src/PadToZone.json %{cfg.targetdir}",
			"{COPY} ../src/Zones.json %{cfg.targetdir}",
			"{COPY} ../src/imgui.ini %{cfg.targetdir}"




		}

	

	filter {"platforms:Win32", "configurations:Release"}
		libdirs {
			path.join(nullspace_win32_dir, "Release/Win32")
		}
		defines {"NDEBUG"}
		optimize "On" 
		postbuildcommands {
			"{COPY} ../../NSLoader/build/bin/Release/Win32/Hardlight.dll %{cfg.targetdir}",
		
			"{COPY} ../src/PadToZone.json %{cfg.targetdir}",
			"{COPY} ../src/Zones.json %{cfg.targetdir}",
			"{COPY} ../src/imgui.ini %{cfg.targetdir}"

		}



	filter {"system:Windows"}
		defines {"_WINDOWS"}




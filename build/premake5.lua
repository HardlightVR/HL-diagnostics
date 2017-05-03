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
	glew_include_dir = "D:/glew-2.0.0/include"
	glfw_include_dir = "D:/glfw-3.2.1/include"
	glm_include_dir = "D:/glm"

	includedirs {
	--	protobuf_incl_dir,
	--	boost_incl_dir,
	--	protobuf_def_incl_dir
	json_include_dir,
	glew_include_dir,
	glfw_include_dir,
	glm_include_dir
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

	



	nullspace_win32_dir = "C:/Users/NullSpace Team/Documents/Visual Studio 2015/Projects/NSLoader/build/bin"
	glfw_win32_dir = "C:/Program Files (x86)/GLFW/lib"
	glew_win32_dir = "D:/glew-2.0.0/lib/Release/Win32"
	glfw_win32_dir2 = "D:/glfw-windows-build/Release"

	pchheader "stdafx.h"
	pchsource "../src/stdafx.cpp"

	
	defines { "GLEW_STATIC", "NOMINMAX"}
	
	filter {"files:**jsoncpp.cpp"}
		flags {'NoPCH'}


	-- input: libprotobuf
	filter {"platforms:Win32"}
		libdirs {
			glfw_win32_dir,
			glew_win32_dir,
			glfw_win32_dir2
		}	
		links {"NSLoader.lib", "glew32s.lib", "glu32.lib", "opengl32.lib", "imm32.lib", "glfw3.lib"}
	


	filter {"platforms:Win32", "configurations:Debug"}
		libdirs {
			path.join(nullspace_win32_dir, "Debug/Win32")
		}

		postbuildcommands {
			"{COPY} ../../NSLoader/build/bin/Debug/Win32/NSLoader.dll %{cfg.targetdir}",
			"{COPY} ../src/VertexShader.txt %{cfg.targetdir}",
			"{COPY} ../src/FragmentShader.txt %{cfg.targetdir}",
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
			"{COPY} ../../NSLoader/build/bin/Release/Win32/NSLoader.dll %{cfg.targetdir}",
			"{COPY} ../src/VertexShader.txt %{cfg.targetdir}",
			"{COPY} ../src/FragmentShader.txt %{cfg.targetdir}",
			"{COPY} ../src/PadToZone.json %{cfg.targetdir}",
			"{COPY} ../src/Zones.json %{cfg.targetdir}",
			"{COPY} ../src/imgui.ini %{cfg.targetdir}"

		}



	filter {"system:Windows"}
		defines {"_WINDOWS"}




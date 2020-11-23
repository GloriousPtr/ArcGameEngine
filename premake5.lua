workspace "Arc"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

project "Arc"
	location "Arc"
	-- DLL --
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	-- Includes --
	includedirs
	{
		"%{prj.name}/vendor/spdlog/include"
	}

	-- Filters --
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"ARC_PLATFORM_WINDOWS",
			"ARC_BUILD_DLL"
		}

		postbuildcommands
		{
			("{Copy} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "ARC_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "ARC_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "ARC_DIST"
		optimize "On"


project "Sandbox"
	location "Sandbox"
	-- Executable --
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	-- Includes --
	includedirs
	{
		"Arc/vendor/spdlog/include",
		"Arc/src"
	}

	-- Dynamic Linking --
	links
	{
		"Arc"
	}

	-- Filters --
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"ARC_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "ARC_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "ARC_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "ARC_DIST"
		optimize "On"


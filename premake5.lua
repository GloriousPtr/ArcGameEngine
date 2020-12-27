workspace "Arc"
	architecture "x86_64"
	startproject "Arc-Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

-- Include directories relavtive to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Arc/vendor/GLFW/include"
IncludeDir["Glad"] = "Arc/vendor/Glad/include"
IncludeDir["ImGui"] = "Arc/vendor/imgui"
IncludeDir["glm"] = "Arc/vendor/glm"
IncludeDir["stb_image"] = "Arc/vendor/stb_image"
IncludeDir["entt"] = "Arc/vendor/entt/include"

group "Dependencies"
	include "Arc/vendor/GLFW"
	include "Arc/vendor/Glad"
	include "Arc/vendor/imgui"

group ""

project "Arc"
	location "Arc"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "arcpch.h"
	pchsource "Arc/src/arcpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	-- Includes --
	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	-- Filters --
	filter "system:windows"
		systemversion "latest"

		defines
		{
			"ARC_PLATFORM_WINDOWS",
			"ARC_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "ARC_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ARC_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ARC_DIST"
		runtime "Release"
		optimize "on"


project "Sandbox"
	location "Sandbox"
	-- Executable --
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"Arc/src",
		"Arc/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	-- Dynamic Linking --
	links
	{
		"Arc"
	}

	-- Filters --
	filter "system:windows"
		systemversion "latest"

		defines
		{
			"ARC_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "ARC_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ARC_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ARC_DIST"
		runtime "Release"
		optimize "on"


project "Arc-Editor"
	location "Arc-Editor"
	-- Executable --
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"Arc/src",
		"Arc/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	-- Dynamic Linking --
	links
	{
		"Arc"
	}

	-- Filters --
	filter "system:windows"
		systemversion "latest"

		defines
		{
			"ARC_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "ARC_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ARC_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ARC_DIST"
		runtime "Release"
		optimize "on"


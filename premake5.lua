include "./vendor/premake/premake_customization/solution_items.lua"

workspace "Arc"
	architecture "x64"
	startproject "Arc-Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	filter "system:windows"

	filter "system:linux"
		toolset "clang"
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

-- Include directories relavtive to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Arc/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Arc/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Arc/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Arc/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Arc/vendor/stb_image"
IncludeDir["entt"] = "%{wks.location}/Arc/vendor/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Arc/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Arc/vendor/ImGuizmo"
IncludeDir["optick"] = "%{wks.location}/Arc/vendor/optick/src"
IncludeDir["box2d"] = "%{wks.location}/Arc/vendor/box2d/include"
IncludeDir["mono"] = "%{wks.location}/Arc/vendor/mono/include"
IncludeDir["miniaudio"] = "%{wks.location}/Arc/vendor/miniaudio"
IncludeDir["icons"] = "%{wks.location}/Arc/vendor/icons/include"
IncludeDir["JoltPhysics"] = "%{wks.location}/Arc/vendor/JoltPhysics/JoltPhysics"
IncludeDir["tinyobj"] = "%{wks.location}/Arc/vendor/tinyobj"
IncludeDir["tinygltf"] = "%{wks.location}/Arc/vendor/tinygltf"

-- Library directories relavtive to root folder (solution directory)
LibDir = {}
filter "system:windows"
	filter "configurations:Debug"
		LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Win64/Debug"
	filter "configurations:Release"
		LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Win64/Release"
	filter "configurations:Dist"
		LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Win64/Release"

filter "system:linux"
	filter "configurations:Debug"
		LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Linux/Debug"
	filter "configurations:Release"
		LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Linux/Debug"
	filter "configurations:Dist"
		LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Linux/Debug"

Lib = {}
filter "system:windows"
	Lib["mono"] = "mono-2.0-sgen.lib"
filter "system:linux"
	Lib["mono"] = "monosgen-2.0"

LibLocation = {}
filter "system:windows"
	LibLocation["Mono"] = "%{LibDir.Mono}/%{Lib.mono}"
filter "system:linux"
	LibLocation["Mono"] = "%{LibDir.Mono}/lib%{Lib.mono}.so"

group "Dependencies"
	include "Arc/vendor/GLFW"
	include "Arc/vendor/Glad"
	include "Arc/vendor/imgui"
	include "Arc/vendor/yaml-cpp"
	include "Arc/vendor/optick"
	include "Arc/vendor/box2d"
	include "Arc/vendor/JoltPhysics"

group ""

include "Arc"
include "Arc-Editor"
include "Arc-ScriptCore"

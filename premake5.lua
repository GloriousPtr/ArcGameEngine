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
		linkoptions { "/ignore:4006" }
		buildoptions { "/bigobj" }
	filter { "system:windows", "toolset:clang" }
		buildoptions { "/showFilenames" }
	filter "system:linux"
		toolset "clang"

-- Library directories relavtive to root folder (solution directory)
LibDir = {}
LibDir["dotnet"] = "%{wks.location}/Arc/vendor/dotnet"

-- Bin directories relavtive to root folder (solution directory)
BinDir = {}

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
IncludeDir["dotnet"] = "%{wks.location}/Arc/vendor/dotnet"
IncludeDir["miniaudio"] = "%{wks.location}/Arc/vendor/miniaudio"
IncludeDir["icons"] = "%{wks.location}/Arc/vendor/icons/include"
IncludeDir["JoltPhysics"] = "%{wks.location}/Arc/vendor/JoltPhysics/JoltPhysics"
IncludeDir["tinyobj"] = "%{wks.location}/Arc/vendor/tinyobj"
IncludeDir["tinygltf"] = "%{wks.location}/Arc/vendor/tinygltf"
IncludeDir["D3D12MA"] = "%{wks.location}/Arc/vendor/D3D12MA/D3D12MemoryAllocator/include"

group "Dependencies"
	include "Arc/vendor/GLFW"
	include "Arc/vendor/Glad"
	include "Arc/vendor/imgui"
	include "Arc/vendor/yaml-cpp"
	include "Arc/vendor/optick"
	include "Arc/vendor/box2d"
	include "Arc/vendor/JoltPhysics"
	include "Arc/vendor/D3D12MA"

group ""

include "Arc"
include "Arc-Editor"
include "Arc-ScriptCore"

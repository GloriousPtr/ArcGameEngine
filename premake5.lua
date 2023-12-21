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

	filter { "action:vs2022" }
		linkoptions { "/ignore:4006" }
		buildoptions { "/bigobj", "/Zc:char8_t-" }
	filter { "action:vs2022", "toolset:clang" }
		buildoptions { "/showFilenames", "/Zc:char8_t-" }
	filter "system:linux"
		toolset "clang"
		buildoptions { "-fno-char8_t" }

	filter { "system:windows", "configurations:Dist" }
		linkoptions { "/SUBSYSTEM:WINDOWS" }

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
IncludeDir["stb"] = "%{wks.location}/Arc/vendor/stb"
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
IncludeDir["magic_enum"] = "%{wks.location}/Arc/vendor/magic_enum"
IncludeDir["EASTL"] = "%{wks.location}/Arc/vendor/EASTL/include"
IncludeDir["EABase"] = "%{wks.location}/Arc/vendor/EABase/include/Common"

group "Dependencies"
	include "Arc/vendor/GLFW"
	include "Arc/vendor/Glad"
	include "Arc/vendor/imgui"
	include "Arc/vendor/yaml-cpp"
	include "Arc/vendor/optick"
	include "Arc/vendor/box2d"
	include "Arc/vendor/JoltPhysics"
	include "Arc/vendor/D3D12MA"
	include "Arc/vendor/EASTL"

group ""

include "Arc"
include "Arc-Editor"
include "Arc-ScriptCore"

include "./vendor/premake/premake_customization/solution_items.lua"

workspace "Arc"
	architecture "x86_64"
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
IncludeDir["assimp"] = "%{wks.location}/Arc/vendor/assimp/assimp/include"
IncludeDir["assimp_config"] = "%{wks.location}/Arc/vendor/assimp/_config_headers"
IncludeDir["assimp_config_assimp"] = "%{wks.location}/Arc/vendor/assimp/_config_headers/assimp"
IncludeDir["optick"] = "%{wks.location}/Arc/vendor/optick/src"
IncludeDir["box2d"] = "%{wks.location}/Arc/vendor/box2d/include"
IncludeDir["mono"] = "%{wks.location}/Arc/vendor/mono/include"
IncludeDir["EABase"] = "%{wks.location}/Arc/vendor/EABase/include/Common"
IncludeDir["EASTL"] = "%{wks.location}/Arc/vendor/EASTL/include"

LibDir = {}
LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/%{cfg.buildcfg}"

Lib = {}
Lib["mono"] = "%{LibDir.Mono}/mono-2.0-sgen.lib"

group "Dependencies"
	include "vendor/premake"
	include "Arc/vendor/GLFW"
	include "Arc/vendor/Glad"
	include "Arc/vendor/imgui"
	include "Arc/vendor/yaml-cpp"
	include "Arc/vendor/assimp"
	include "Arc/vendor/optick"
	include "Arc/vendor/box2d"
	include "Arc/vendor/EASTL"

group ""

include "Arc"
include "Arc-Editor"
include "Arc-ScriptCore"

include "Sandbox"

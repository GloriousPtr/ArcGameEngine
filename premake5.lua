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
IncludeDir["assimp"] = "%{wks.location}/Arc/vendor/assimp/include"
IncludeDir["optick"] = "%{wks.location}/Arc/vendor/optick/src"

group "Dependencies"
	include "vendor/premake"
	include "Arc/vendor/GLFW"
	include "Arc/vendor/Glad"
	include "Arc/vendor/imgui"
	include "Arc/vendor/yaml-cpp"
	include "Arc/vendor/assimp"
	include "Arc/vendor/optick"

group ""

include "Arc"
include "Sandbox"
include "Arc-Editor"

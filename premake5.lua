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

	files
	{
		"%{wks.location}/Arc/vendor/glm/util/glm.natvis",
		"%{wks.location}/Arc/vendor/imgui/misc/debuggers/imgui.natvis",
		"%{wks.location}/Arc/vendor/imgui/misc/debuggers/imgui.natstepfilter",
		"%{wks.location}/Arc/vendor/yaml-cpp/src/contrib/yaml-cpp.natvis",
		"%{IncludeDir.JoltPhysics}/Jolt/Jolt.natvis",
		"%{IncludeDir.D3D12MA}/../src/D3D12MemAlloc.natvis",
		"%{IncludeDir.entt}/../natvis/config.natvis",
		"%{IncludeDir.entt}/../natvis/container.natvis",
		"%{IncludeDir.entt}/../natvis/core.natvis",
		"%{IncludeDir.entt}/../natvis/entity.natvis",
		"%{IncludeDir.entt}/../natvis/graph.natvis",
		"%{IncludeDir.entt}/../natvis/locator.natvis",
		"%{IncludeDir.entt}/../natvis/meta.natvis",
		"%{IncludeDir.entt}/../natvis/platform.natvis",
		"%{IncludeDir.entt}/../natvis/poly.natvis",
		"%{IncludeDir.entt}/../natvis/process.natvis",
		"%{IncludeDir.entt}/../natvis/resource.natvis",
		"%{IncludeDir.entt}/../natvis/signal.natvis",
	}

	filter "system:windows"
		nuget
		{
			"Microsoft.Direct3D.D3D12:1.608.3",
			"Microsoft.Direct3D.DXC:1.7.2212.36"
		}
		linkoptions { "/ignore:4006" }
	filter "system:linux"
		toolset "clang"

-- Library directories relavtive to root folder (solution directory)
LibDir = {}
LibDir["Pix"] = "%{wks.location}/Arc/vendor/WinPixRuntime/bin/x64"
filter "configurations:Debug"
	LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Debug"
filter "configurations:Release"
	LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Release"
filter "configurations:Dist"
	LibDir["Mono"] = "%{wks.location}/Arc/vendor/mono/lib/Release"

-- Bin directories relavtive to root folder (solution directory)
BinDir = {}
BinDir["Pix"] = "%{wks.location}/Arc/vendor/WinPixRuntime/bin/x64"
filter "configurations:Debug"
	BinDir["Mono"] = "%{wks.location}/Arc/vendor/mono/bin/Debug"
filter "configurations:Release"
	BinDir["Mono"] = "%{wks.location}/Arc/vendor/mono/bin/Release"
filter "configurations:Dist"
	BinDir["Mono"] = "%{wks.location}/Arc/vendor/mono/bin/Release"

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
IncludeDir["D3D12MA"] = "%{wks.location}/Arc/vendor/D3D12MA/D3D12MemoryAllocator/include"
IncludeDir["Pix"] = "%{wks.location}/Arc/vendor/WinPixRuntime/Include"

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

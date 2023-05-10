project "Arc"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "extra"
	externalwarnings "off"
	rtti "off"
	postbuildmessage ""

	flags { "FatalWarnings" }

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "arcpch.h"
	pchsource "src/arcpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",

		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",

		"vendor/**.natvis",
		"vendor/**.natstepfilter",
	}

	defines
	{
		"GLFW_INCLUDE_NONE",
		"SPDLOG_USE_STD_FORMAT",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
	}

	includedirs
	{
		"src"
	}

	externalincludedirs
	{
		"vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.optick}",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.dotnet}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.icons}",
		"%{IncludeDir.JoltPhysics}",
		"%{IncludeDir.tinyobj}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.magic_enum}",
		"%{IncludeDir.EABase}",
		"%{IncludeDir.EASTL}",
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"optick",
		"box2d",
		"JoltPhysics",
		"EASTL",
	}

	dependson
	{
		"Arc-ScriptCore",
	}

	postbuildcommands
	{
		-- Nethost
		'{ECHO} ====== Copying Nethost ======',
		'{COPYFILE} %{LibDir.dotnet}/nethost.dll "%{cfg.targetdir}"',
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		externalincludedirs
		{
			"%{IncludeDir.D3D12MA}",
		}
		links
		{
			"D3D12MA",

			"%{LibDir.dotnet}/nethost.dll",

			"opengl.dll",

			-- DirectX
			"dxguid.dll",
			"d3d12.dll",
			"dxgi.dll",
		}
		nuget
		{
			"Microsoft.Direct3D.D3D12:1.608.3",
			"Microsoft.Direct3D.DXC:1.7.2212.36",
			"directxtk12_desktop_2019:2023.2.7.1",
			"WinPixEventRuntime:1.0.230302001",
		}

	filter "system:linux"
		pic "On"
		systemversion "latest"
		buildoptions { "`pkg-config --cflags gtk+-3.0`" }
		linkoptions { "`pkg-config --libs gtk+-3.0`" }
		links
		{
			"GL:shared",
			"dl:shared",
		}

	filter "configurations:Debug"
		defines "ARC_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ARC_RELEASE"
		runtime "Release"
		optimize "speed"

	filter "configurations:Dist"
		defines "ARC_DIST"
		runtime "Release"
		optimize "speed"
		symbols "off"

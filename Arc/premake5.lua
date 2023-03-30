project "Arc"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "extra"
	externalwarnings "off"
	rtti "off"
	postbuildmessage "================ Post-Build: Copying dependencies ================"

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
	}

	defines
	{
		"GLFW_INCLUDE_NONE"
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
		"%{IncludeDir.mono}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.icons}",
		"%{IncludeDir.JoltPhysics}",
		"%{IncludeDir.tinyobj}",
		"%{IncludeDir.tinygltf}",
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
		"Arc-ScriptCore",
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/bigobj" }
		externalincludedirs
		{
			"%{IncludeDir.D3D12MA}",
			"%{IncludeDir.Pix}",
		}
		links
		{
			"D3D12MA",

			"%{LibDir.Mono}/mono-2.0-sgen.dll",
			"opengl.dll",

			-- DirectX
			"dxguid.dll",
			"d3d12.dll",
			"dxgi.dll",

			-- Pix
			"%{LibDir.Pix}/WinPixEventRuntime.dll",
		}

	filter "system:linux"
		pic "On"
		systemversion "latest"
		buildoptions { "`pkg-config --cflags gtk+-3.0`" }
		linkoptions { "`pkg-config --libs gtk+-3.0`" }
		links
		{
			"monosgen-2.0:shared",
			"GL:shared",
			"dl:shared",
		}

	filter "configurations:Debug"
		defines "ARC_DEBUG"
		runtime "Debug"
		symbols "on"
		postbuildcommands
		{
			'{COPYFILE} "%{BinDir.Mono}/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
		}

	filter "configurations:Release"
		defines "ARC_RELEASE"
		runtime "Release"
		optimize "speed"
		postbuildcommands
		{
			'{COPYFILE} "%{BinDir.Mono}/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
		}

	filter "configurations:Dist"
		defines "ARC_DIST"
		runtime "Release"
		optimize "speed"
		symbols "off"
		postbuildcommands
		{
			'{COPYFILE} "%{BinDir.Mono}/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
		}

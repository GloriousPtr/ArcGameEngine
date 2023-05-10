project "Arc-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "extra"
	externalwarnings "off"
	rtti "off"
	buildmessage ""
	postbuildmessage ""

	flags { "FatalWarnings" }

	binDir = "%{wks.location}/bin/" .. outputdir
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",

		"%{wks.location}/Arc/vendor/**.natvis",
		"%{wks.location}/Arc/vendor/**.natstepfilter",
	}

	defines
	{
		"SPDLOG_USE_STD_FORMAT",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
	}

	includedirs
	{
		"src",
		"%{wks.location}/Arc/src",
		"%{wks.location}/Arc/vendor"
	}

	externalincludedirs
	{
		"%{wks.location}/Arc/vendor/spdlog/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.optick}",
		"%{IncludeDir.icons}",
		"%{IncludeDir.magic_enum}",
		"%{IncludeDir.EABase}",
		"%{IncludeDir.EASTL}",
	}
	
	links
	{
		"Arc",
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

	buildcommands
	{
		'{ECHO} ====== Copying Resource Files ======',
		'{COPYDIR} "../vendor" "%{binDir}"/vendor',
		'{COPYDIR} "assets" "%{cfg.targetdir}"/assets',
		'{COPYDIR} "Resources" "%{cfg.targetdir}"/Resources',
		'{COPYFILE} "imgui.ini" "%{cfg.targetdir}"',
	}
	
	buildoutputs
	{
		"%{cfg.targetdir}/assets",
		"%{cfg.targetdir}/Resources",
	}

	postbuildcommands
	{
		-- Nethost
		'{ECHO} ====== Copying Nethost ======',
		'{COPYFILE} %{LibDir.dotnet}/nethost.dll "%{cfg.targetdir}"',
	}

	filter "system:windows"
		systemversion "latest"
		links
		{
			"%{LibDir.dotnet}/nethost.lib",

			"opengl.dll",

			-- DirectX
			"dxguid.lib",
			"d3d12.lib",
			"dxgi.lib",
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
		linkoptions { "`pkg-config --libs gtk+-3.0`" }
		links
		{
			"GL:shared",
			"dl:shared"
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

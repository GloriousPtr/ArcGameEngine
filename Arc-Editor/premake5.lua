project "Arc-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "default"
	externalwarnings "off"

	binDir = "%{wks.location}/bin/" .. outputdir
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",

		"%{IncludeDir.glm}/util/glm.natvis",
		"%{IncludeDir.yaml_cpp}/../src/contrib/yaml-cpp.natvis",
		"%{IncludeDir.ImGui}/misc/debuggers/imgui.natvis",
	}

	includedirs
	{
		"%{wks.location}/Arc/vendor/spdlog/include",
		"%{wks.location}/Arc/src",
		"%{wks.location}/Arc/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.optick}",
		"%{IncludeDir.icons}",
	}

	links
	{
		"Arc",
	}

	defines
	{
		"SPDLOG_USE_STD_FORMAT"
	}

	buildoptions { "/utf-8" }

	filter "system:windows"
		systemversion "latest"
		postbuildmessage "Post-Build: Copying dependencies..."
		postbuildcommands
		{
			'{COPY} "../vendor" "%{binDir}"/vendor',
			'{COPY} "../Arc-Editor/assets" "%{cfg.targetdir}"/assets',
			'{COPY} "../Arc-Editor/mono" "%{cfg.targetdir}"/mono',
			'{COPY} "../Arc-Editor/Resources" "%{cfg.targetdir}"/Resources',
			'{COPY} "../Arc-Editor/imgui.ini" "%{cfg.targetdir}"',
		}

	filter "configurations:Debug"
		defines "ARC_DEBUG"
		runtime "Debug"
		symbols "on"
		postbuildcommands
		{
			'{COPY} "../Arc/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}

	filter "configurations:Release"
		defines "ARC_RELEASE"
		runtime "Release"
		optimize "speed"
		postbuildcommands
		{
			'{COPY} "../Arc/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}

	filter "configurations:Dist"
		defines "ARC_DIST"
		runtime "Release"
        optimize "speed"
		symbols "off"
		postbuildcommands
		{
			'{COPY} "../Arc/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}

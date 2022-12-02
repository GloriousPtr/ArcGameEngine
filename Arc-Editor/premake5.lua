project "Arc-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	binDir = "%{wks.location}/bin/" .. outputdir
	targetdir ("%{binDir}/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",

		"%{IncludeDir.EASTL}/../doc/EASTL.natvis",
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
		"%{IncludeDir.EABase}",
		"%{IncludeDir.EASTL}",
		"%{IncludeDir.icons}",
	}

	links
	{
		"Arc",
	}

	filter "system:windows"
		systemversion "latest"
		postbuildcommands
		{
			'{COPY} "../Arc-ScriptCore" "%{binDir}"/Arc-ScriptCore',
			'{COPY} "../vendor" "%{binDir}"/vendor',
			'{COPY} "../Sandbox" "%{binDir}"/Sandbox',
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

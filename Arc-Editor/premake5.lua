project "Arc-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "default"
	externalwarnings "off"
	rtti "off"
	characterset "Unicode"

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
	}

	libdirs
	{
		"%{LibDir.Mono}"
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
		"Arc-ScriptCore",

		"%{Lib.mono}:shared",
		"GL:shared",
	}

	postbuildmessage "================ Post-Build: Copying dependencies ================"
	postbuildcommands
	{
		'{COPY} "%{LibLocation.Mono}" "%{cfg.targetdir}"'
	}

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

	filter "configurations:Release"
		defines "ARC_RELEASE"
		runtime "Release"
		optimize "speed"

	filter "configurations:Dist"
		defines "ARC_DIST"
		runtime "Release"
        optimize "speed"
		symbols "off"

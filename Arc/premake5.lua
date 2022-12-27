project "Arc"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "default"
	externalwarnings "off"
	rtti "off"

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

		"%{IncludeDir.glm}/util/glm.natvis",
		"%{IncludeDir.yaml_cpp}/../src/contrib/yaml-cpp.natvis",
		"%{IncludeDir.JoltPhysics}/Jolt/Jolt.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/config.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/container.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/core.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/entity.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/graph.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/locator.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/meta.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/platform.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/poly.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/process.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/resource.natvis",
		"%{IncludeDir.JoltPhysics}/entt/natvis/signal.natvis",
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

	libdirs
	{
		"%{LibDir.Mono}"
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

		"%{Lib.mono}:shared",
		"GL:shared",
		"dl:shared"
	}

	postbuildmessage "================ Post-Build: Copying dependencies ================"
	postbuildcommands
	{
		'{COPY} "%{LibLocation.Mono}" "%{cfg.targetdir}"'
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"
		systemversion "latest"

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

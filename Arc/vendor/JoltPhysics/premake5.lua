project "JoltPhysics"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
	}

	files
	{
		"JoltPhysics/Jolt/**.h",
		"JoltPhysics/Jolt/**.cpp",
		"JoltPhysics/Jolt/**.inl",
		"JoltPhysics/Jolt/Jolt.natvis"
	}

	includedirs
	{
		"JoltPhysics",
		"%{IncludeDir.optick}"
	}

	links
	{
		"optick"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "speed"

    filter "configurations:Dist"
		runtime "Release"
		optimize "speed"
        symbols "off"

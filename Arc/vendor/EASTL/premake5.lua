project "EASTL"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/**.h",
		"include/**.cpp",
		"source/**.h",
		"source/**.cpp"
	}

	includedirs 
	{
		"include",
		"source",
		"../EABase/include/Common",
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++20"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++20"

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

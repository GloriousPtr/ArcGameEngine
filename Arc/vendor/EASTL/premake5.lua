project "EASTL"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"EASTL/include/**.h",
		"EASTL/include/**.cpp",
		"EASTL/source/**.h",
		"EASTL/source/**.cpp"
	}

	includedirs 
	{
		"EASTL/include",
		"EASTL/source",
		"../EABase/EABase/include/Common",
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

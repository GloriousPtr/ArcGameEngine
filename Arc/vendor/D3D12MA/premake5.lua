project "D3D12MA"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"D3D12MemoryAllocator/src/Common.h",
		"D3D12MemoryAllocator/src/Common.cpp",
		"D3D12MemoryAllocator/src/D3D12MemAlloc.cpp",
	}

	includedirs
	{
		"D3D12MemoryAllocator/include",
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

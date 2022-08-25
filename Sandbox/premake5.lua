local ArcRootDir = '../'
include (ArcRootDir .. "/vendor/premake/premake_customization/solution_items.lua")

workspace "Sandbox"
    architecture "x86_64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Sandbox"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.8"
    csversion "10.0"

    targetdir ("Assemblies")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Assets/**.cs"
    }

    links
    {
        "Arc-ScriptCore"
    }

    filter "configurations:Debug"
        optimize "Off"
        symbols "Full"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"
        
    filter "configurations:Dist"
        optimize "Full"
        symbols "Off"

group "Hazel"
    include (ArcRootDir .. "/Arc-ScriptCore")
group ""

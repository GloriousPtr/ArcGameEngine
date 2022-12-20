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

project "Sandbox"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.8"
    csversion "10.0"

    targetdir ("Binaries")
	objdir ("Intermediates")

    files
    {
        "Assets/**.cs"
    }

    links
    {
        ArcRootDir .. "/Arc-Editor/Resources/Scripts/Arc-ScriptCore.dll"
    }

    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"
        defines { "DEBUG" }

    filter "configurations:Release"
        optimize "On"
        symbols "Default"
        defines { "RELEASE" }
        
    filter "configurations:Dist"
        optimize "Full"
        symbols "Off"
        defines { "DIST" }

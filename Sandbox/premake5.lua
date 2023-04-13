local ArcRootDir = '../'
include (ArcRootDir .. "/vendor/premake/premake_customization/solution_items.lua")

workspace "Sandbox"
	architecture "x64"
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
    dotnetframework "net7.0"
    csversion "11.0"

    targetdir ("Binaries")
	objdir ("Intermediates")

    files
    {
        "Assets/**.cs"
    }

    links
    {
        _WORKING_DIR .. "/Resources/Scripts/Arc-ScriptCore.dll"
    }

    filter "configurations:Debug"
        optimize "off"
        symbols "default"
        defines { "DEBUG" }

    filter "configurations:Release"
        optimize "on"
        symbols "default"
        defines { "RELEASE" }
        
    filter "configurations:Dist"
        optimize "full"
        symbols "off"
        defines { "DIST" }

require "vstudio"

function platformsElement(cfg)
	_p(2,'<Platforms>x64</Platforms>\n    <EnableDynamicLoading>true</EnableDynamicLoading>\n    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>\n    <Configurations>Debug;Release;Dist</Configurations>')
end

premake.override(premake.vstudio.cs2005.elements, "projectProperties", function (oldfn, cfg)
	return table.join(oldfn(cfg), {
	platformsElement,
	})
end)

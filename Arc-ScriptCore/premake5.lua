project "Arc-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "net7.0"
    csversion "11.0"

	flags { "FatalWarnings" }
    
	targetdir ("../Arc-Editor/Resources/Scripts")
	objdir ("../Arc-Editor/Resources/Scripts/Intermediates")

    files
    {
        "src/**.cs",
        "vendor/**cs",
    }

    defines
    {
        "JETBRAINS_ANNOTATIONS"
    }

    filter "configurations:Debug"
        optimize "off"
        symbols "default"

    filter "configurations:Release"
        optimize "on"
        symbols "default"
        
    filter "configurations:Dist"
        optimize "full"
        symbols "off"

require "vstudio"

function platformsElement(cfg)
	_p(2,'<Platforms>x64</Platforms>\n    <EnableDynamicLoading>true</EnableDynamicLoading>\n    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>\n    <Configurations>Debug;Release;Dist</Configurations>')
end

premake.override(premake.vstudio.cs2005.elements, "projectProperties", function (oldfn, cfg)
	return table.join(oldfn(cfg), {
	platformsElement,
	})
end)

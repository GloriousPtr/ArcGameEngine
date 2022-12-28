project "Arc-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.8"
    csversion "7.2"
    toolset "msc"
    
	targetdir ("../Arc-Editor/Resources/Scripts")
	objdir ("../Arc-Editor/Resources/Scripts/Intermediates")

    files
    {
        "src/**.cs",
        "vendor/**cs",
    }

    links
    {
        "System.Core"
    }

    defines
    {
        "JETBRAINS_ANNOTATIONS"
    }

    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"
        
    filter "configurations:Dist"
        optimize "Full"
        symbols "Off"

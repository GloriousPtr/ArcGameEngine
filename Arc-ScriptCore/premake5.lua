project "Arc-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.8"
    csversion "10.0"
    
	targetdir ("../Arc-Editor/Resources/Scripts")
	objdir ("../Arc-Editor/Resources/Scripts/Intermediates")

    files
    {
        "src/**.cs"
    }

    links
    {
        "Microsoft.Bcl.HashCode"
    }

    nuget
    {
        "JetBrains.Annotations:2022.3.1"
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

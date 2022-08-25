project "Arc-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.8"
    csversion "10.0"
    
	targetdir ("%{wks.location}/Arc-Editor/Resources/Scripts")
	objdir ("%{wks.location}/Arc-Editor/Resources/Scripts/Intermediates")

    files
    {
        "src/**.cs"
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

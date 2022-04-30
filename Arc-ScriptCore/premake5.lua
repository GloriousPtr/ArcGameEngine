project "Arc-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    
	targetdir ("%{wks.location}/Arc-Editor/Resources/Scripts")
	objdir ("%{wks.location}/Arc-Editor/Resources/Scripts/Intermediates")

    files
    {
        "src/**.cs"
    }

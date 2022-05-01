project "Sandbox"
    kind "SharedLib"
    language "C#"

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

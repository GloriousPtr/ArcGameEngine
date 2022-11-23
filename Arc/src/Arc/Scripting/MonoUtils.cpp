#include "arcpch.h"
#include "MonoUtils.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/object.h>

#include "Arc/Core/Filesystem.h"
#include "ScriptEngine.h"

namespace ArcEngine
{
	MonoAssembly* MonoUtils::LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPdb)
	{
		ScopedBuffer fileBuffer = Filesystem::ReadFileBinary(assemblyPath);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileBuffer.As<char>(), (uint32_t)fileBuffer.Size(), 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			ARC_CORE_ERROR("Could not load mono image from assembly.\n{}", errorMessage);
			return nullptr;
		}

		ARC_CORE_TRACE("Loaded Assembly Image: {}", assemblyPath);
		if (loadPdb)
		{
			std::filesystem::path pdbPath = assemblyPath;
			pdbPath.replace_extension(".pdb");

			if (std::filesystem::exists(pdbPath))
			{
				ScopedBuffer pdbFileData = Filesystem::ReadFileBinary(pdbPath);
				mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), (int32_t)pdbFileData.Size());
				ARC_CORE_TRACE("Loaded PDB: {}", pdbPath);
			}
		}

		std::string pathString = assemblyPath.string();
		MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
		mono_image_close(image);

		return assembly;
	}

	bool MonoUtils::CheckMonoError(MonoError& error)
	{
		ARC_PROFILE_SCOPE();

		bool hasError = !mono_error_ok(&error);

		if (hasError)
		{
			uint16_t errorCode = mono_error_get_error_code(&error);
			const char* errorMessage = mono_error_get_message(&error);

			ARC_CORE_ERROR("Mono Error");
			ARC_CORE_ERROR("Error Code: {0}\nError Message: {1}", errorCode, errorMessage);
			mono_error_cleanup(&error);
			return true;
		}

		return false;
	}

	eastl::string MonoUtils::MonoStringToUTF8(MonoString* monoString)
	{
		ARC_PROFILE_SCOPE();

		if (monoString == nullptr || mono_string_length(monoString) == 0)
			return "";

		MonoError error;
		char* utf8 = mono_string_to_utf8_checked(monoString, &error);
		if (CheckMonoError(error))
			return "";

		eastl::string result(utf8);
		mono_free(utf8);
		return result;
	}

	MonoString* MonoUtils::UTF8ToMonoString(const eastl::string& str)
	{
		ARC_PROFILE_SCOPE();

		return mono_string_new(ScriptEngine::GetDomain(), str.c_str());
	}
}

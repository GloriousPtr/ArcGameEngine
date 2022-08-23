#include "arcpch.h"
#include "MonoUtils.h"

#include <fstream>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

#include "ScriptEngine.h"

namespace ArcEngine
{
	static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return nullptr;
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();

		if (size == 0)
		{
			// File is empty
			return nullptr;
		}

		char* buffer = new char[size];
		stream.read((char*)buffer, size);
		stream.close();

		*outSize = (uint32_t)size;
		return buffer;
	}

	MonoAssembly* MonoUtils::LoadMonoAssembly(const std::filesystem::path& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			// Log some error message using the errorMessage data
			return nullptr;
		}

		std::string pathString = assemblyPath.string();
		MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
		mono_image_close(image);

		// Don't forget to free the file data
		delete[] fileData;

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

#pragma once

typedef union _MonoError MonoError;
typedef struct _MonoString MonoString;
typedef struct _MonoAssembly MonoAssembly;

namespace ArcEngine
{
	class MonoUtils
	{
	public:
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath);

		static bool CheckMonoError(MonoError& error);
		static eastl::string MonoStringToUTF8(MonoString* monoString);
		static MonoString* UTF8ToMonoString(const eastl::string& monoString);
	};
}

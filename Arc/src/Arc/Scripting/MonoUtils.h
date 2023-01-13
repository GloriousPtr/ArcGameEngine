#pragma once

typedef union _MonoError MonoError;
typedef struct _MonoString MonoString;
typedef struct _MonoAssembly MonoAssembly;

namespace ArcEngine
{
	class MonoUtils
	{
	public:
		[[nodiscard]] static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPdb);

		[[nodiscard]] static bool CheckMonoError(MonoError& error);
		[[nodiscard]] static std::string MonoStringToUTF8(MonoString* monoString);
		[[nodiscard]] static MonoString* UTF8ToMonoString(const std::string& str);
	};
}

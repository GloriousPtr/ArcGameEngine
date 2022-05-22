#pragma once

typedef union _MonoError MonoError;
typedef struct _MonoString MonoString;

namespace ArcEngine
{
	class MonoUtils
	{
	public:
		static bool CheckMonoError(MonoError& error);
		static eastl::string MonoStringToUTF8(MonoString* monoString);
		static MonoString* UTF8ToMonoString(const eastl::string& monoString);
	};
}

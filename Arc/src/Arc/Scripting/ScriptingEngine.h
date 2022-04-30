#pragma once

typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;
typedef struct _MonoClass MonoClass;
typedef struct _MonoMethod MonoMethod;

namespace ArcEngine
{
	class ScriptingEngine
	{
	public:
		static void Init(const char* assemblyPath);
		static void Shutdown();

		static bool HasClass(const char* className);
		static void* MakeInstance(const char* className);
		static void Call(void* instance, const char* className, const char* methodSignature, void** args);
		
		static void CacheMethodIfAvailable(const char* className, const char* methodSignature);
		static void CacheClassIfAvailable(const char* className);

		static MonoMethod* GetCachedMethodIfAvailable(const char* className, const char* methodSignature);
		static MonoClass* GetCachedClassIfAvailable(const char* className);

		static MonoMethod* GetMethod(const char* className, const char* methodSignature);
		static MonoClass* GetClass(const char* className);

	private:

		static MonoDomain* s_MonoDomain;
		static MonoAssembly* s_ScriptCoreAssembly;
		static MonoImage* s_ScriptCoreImage;

		static std::unordered_map<std::string, MonoClass*> s_ClassMap;
		static std::unordered_map<std::string, MonoMethod*> s_MethodMap;
	};
}

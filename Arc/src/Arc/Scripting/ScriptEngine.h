#pragma once

#include "Field.h"
#include "Arc/Scene/Scene.h"

typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;
typedef struct _MonoClass MonoClass;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoProperty MonoProperty;
typedef struct _MonoClassField MonoClassField;

namespace ArcEngine
{
	class ScriptEngine
	{
	public:
		static void Init(const char* coreAssemblyPath);
		static void Shutdown();
		static void LoadCoreAssembly(const char* path);
		static void LoadClientAssembly(const char* path);

		static bool HasClass(const char* className);
		static void* MakeInstance(const char* className);
		static void Call(void* instance, const char* className, const char* methodSignature, void** args);
		static void SetProperty(void* instance, void* property, void** params);
		
		static void CacheMethodIfAvailable(const char* className, const char* methodSignature);
		static void CacheClassIfAvailable(const char* className);

		static MonoMethod* GetCachedMethodIfAvailable(const char* className, const char* methodSignature);
		static MonoClass* GetCachedClassIfAvailable(const char* className);

		static MonoMethod* GetMethod(const char* className, const char* methodSignature);
		static MonoClass* GetClass(const char* className);
		static MonoProperty* GetProperty(const char* className, const char* propertyName);
		static std::unordered_map<std::string, Field>* GetFields(const char* className);

		static void SetScene(Scene* scene) { s_CurrentScene = scene; }
		static Scene* GetScene() { return s_CurrentScene; }

		static MonoImage* GetCoreAssemblyImage() { return s_ScriptCoreImage; }

	private:
		static MonoClass* CreateClass(MonoImage* image, const char* namespaceName, const char* className, const char* fullName);

	private:

		static MonoDomain* s_MonoDomain;
		static MonoAssembly* s_ScriptCoreAssembly;
		static MonoAssembly* s_ScriptClientAssembly;
		static MonoImage* s_ScriptCoreImage;
		static MonoImage* s_ScriptClientImage;

		static std::unordered_map<std::string, MonoClass*> s_ClassMap;
		static std::unordered_map<std::string, MonoMethod*> s_MethodMap;
		static std::unordered_map<std::string, MonoProperty*> s_PropertyMap;
		static std::unordered_map<std::string, std::unordered_map<std::string, Field>> s_FieldMap;

		static Scene* s_CurrentScene;
	};
}

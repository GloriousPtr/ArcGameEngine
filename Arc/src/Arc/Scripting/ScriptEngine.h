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
	using GCHandle = void*;

	class ScriptEngine
	{
	public:
		static void Init(const char* coreAssemblyPath);
		static void Shutdown();
		static void LoadCoreAssembly(const char* path);
		static void LoadClientAssembly(const char* path);

		static bool HasClass(const char* className);
		static GCHandle MakeReference(const char* className);
		static GCHandle CopyStrongReference(GCHandle className);
		static void ReleaseObjectReference(const GCHandle handle);
		static void Call(GCHandle handle, const char* className, const char* methodSignature, void** args);
		static void SetProperty(GCHandle handle, void* property, void** params);
		
		static void CacheMethodIfAvailable(const char* className, const char* methodSignature);
		static void CacheClassIfAvailable(const char* className);

		static MonoMethod* GetCachedMethodIfAvailable(const char* className, const char* methodSignature);
		static MonoClass* GetCachedClassIfAvailable(const char* className);

		static MonoMethod* GetMethod(const char* className, const char* methodSignature);
		static MonoClass* GetClass(const char* className);
		static MonoProperty* GetProperty(const char* className, const char* propertyName);
		static eastl::unordered_map<eastl::string, Field>* GetFields(const char* className);

		static void SetScene(Scene* scene) { s_CurrentScene = scene; }
		static Scene* GetScene() { return s_CurrentScene; }

		static MonoImage* GetCoreAssemblyImage() { return s_ScriptCoreImage; }
		static MonoDomain* GetDomain() { return s_MonoDomain; }

	private:
		static MonoClass* CreateClass(MonoImage* image, const char* namespaceName, const char* className, const char* fullName);

	private:

		static MonoDomain* s_MonoDomain;
		static MonoAssembly* s_ScriptCoreAssembly;
		static MonoAssembly* s_ScriptClientAssembly;
		static MonoImage* s_ScriptCoreImage;
		static MonoImage* s_ScriptClientImage;

		static eastl::unordered_map<eastl::string, MonoClass*> s_ClassMap;
		static eastl::unordered_map<eastl::string, MonoMethod*> s_MethodMap;
		static eastl::unordered_map<eastl::string, MonoProperty*> s_PropertyMap;
		static eastl::unordered_map<eastl::string, eastl::unordered_map<eastl::string, Field>> s_FieldMap;

		static Scene* s_CurrentScene;
	};
}

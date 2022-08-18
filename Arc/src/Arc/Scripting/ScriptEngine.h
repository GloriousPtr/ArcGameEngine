#pragma once

#include <EASTL/unordered_map.h>

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

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(MonoClass* monoClass);
		ScriptClass(const eastl::string& classNamespace, const eastl::string& className);
		
		GCHandle Instantiate();
		MonoMethod* GetMethod(const eastl::string& signature);
		GCHandle InvokeMethod(GCHandle gcHandle, MonoMethod* method, void** params = nullptr);

		eastl::unordered_map<eastl::string, Field>& GetFields();
		MonoProperty* GetProperty(const char* className, const char* propertyName);
		void SetProperty(GCHandle gcHandle, void* property, void** params);

	private:
		void LoadFields();

	private:
		eastl::string m_ClassNamespace;
		eastl::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
		eastl::unordered_map<eastl::string, Field> m_Fields;
		eastl::unordered_map<eastl::string, MonoProperty*> m_Properties;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptInstance>, Entity entity);
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);
		void InvokeOnDestroy();

		GCHandle GetHandle() { return m_Handle; }

	private:
		Ref<ScriptClass> m_ScriptClass;

		GCHandle m_Handle = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		MonoMethod* m_OnDestroyMethod = nullptr;
	};

	class ScriptEngine
	{
	public:
		static void Init(const char* coreAssemblyPath);
		static void Shutdown();
		static void LoadCoreAssembly(const char* path);
		static void LoadClientAssembly(const char* path);
		static void LoadAssemblyClasses(MonoAssembly* assembly);

		static MonoDomain* GetDomain();
		static MonoImage* GetCoreAssemblyImage();

		static GCHandle CreateInstance(Entity entity, const eastl::string& name);
		static GCHandle CreateInstanceRuntime(Entity entity, const eastl::string& name);
		static Ref<ScriptInstance>& GetInstance(Entity entity, const eastl::string& name);
		static Ref<ScriptInstance>& GetInstanceRuntime(Entity entity, const eastl::string& name);

		static void ReleaseObjectReference(const GCHandle handle);
		
		static void SetProperty(GCHandle handle, void* property, void** params);
		static MonoProperty* GetProperty(const char* className, const char* propertyName);

		static eastl::unordered_map<eastl::string, Ref<ScriptClass>>& GetClasses();
		static eastl::unordered_map<eastl::string, Field>& GetFields(const char* className);

		static void SetScene(Scene* scene) { s_CurrentScene = scene; }
		static Scene* GetScene() { return s_CurrentScene; }

	private:

		static Scene* s_CurrentScene;
	};
}

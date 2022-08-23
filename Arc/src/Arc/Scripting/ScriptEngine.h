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

		MonoProperty* GetProperty(const char* className, const char* propertyName);
		void SetProperty(GCHandle gcHandle, void* property, void** params);

	private:
		void LoadFields();

	private:

		friend class ScriptInstance;

		eastl::string m_ClassNamespace;
		eastl::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
		eastl::map<eastl::string, MonoClassField*> m_Fields;
		eastl::map<eastl::string, MonoProperty*> m_Properties;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(ScriptInstance*, UUID entityID);
		ScriptInstance(Ref<ScriptClass> scriptClass, UUID entityID);

		~ScriptInstance();

		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);
		void InvokeOnDestroy();

		GCHandle GetHandle() { return m_Handle; }
		eastl::map<eastl::string, Ref<Field>>& GetFields() { return m_Fields; }

		void Invalidate(Ref<ScriptClass> scriptClass, UUID entityID);

	private:
		void LoadFields();

	private:
		Ref<ScriptClass> m_ScriptClass;

		GCHandle m_Handle = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		MonoMethod* m_OnDestroyMethod = nullptr;

		eastl::map<eastl::string, Ref<Field>> m_Fields;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();
		static void LoadCoreAssembly();
		static void LoadClientAssembly();
		static void ReloadAppDomain();
		static void LoadAssemblyClasses(MonoAssembly* assembly);

		static MonoDomain* GetDomain();
		static MonoImage* GetCoreAssemblyImage();

		static void OnRuntimeBegin();
		static void OnRuntimeEnd();

		static ScriptInstance* CreateInstance(Entity entity, const eastl::string& name);
		static ScriptInstance* GetInstance(Entity entity, const eastl::string& name);
		static void RemoveInstance(Entity entity, const eastl::string& name);
		
		static void SetProperty(GCHandle handle, void* property, void** params);
		static MonoProperty* GetProperty(const char* className, const char* propertyName);

		static bool HasClass(const eastl::string& className);
		static eastl::unordered_map<eastl::string, Ref<ScriptClass>>& GetClasses();
		static eastl::map<eastl::string, Ref<Field>>& GetFields(Entity entity, const char* className);

		static void SetScene(Scene* scene) { s_CurrentScene = scene; }
		static Scene* GetScene() { return s_CurrentScene; }

	private:

		static Scene* s_CurrentScene;
	};
}

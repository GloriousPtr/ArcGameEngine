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
		MonoMethod* GetMethod(const char* methodName, uint32_t parameterCount);
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

	struct Collision2DData
	{
		UUID EntityID = 0;
		glm::vec2 RelativeVelocity = { 0.0f, 0.0f };
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
		void InvokeOnCollisionEnter2D(Collision2DData& other);
		void InvokeOnCollisionExit2D(Collision2DData& other);
		void InvokeOnSensorEnter2D(Collision2DData& other);
		void InvokeOnSensorExit2D(Collision2DData& other);

		GCHandle GetHandle() { return m_Handle; }
		eastl::map<eastl::string, Ref<Field>>& GetFields() { return m_Fields; }

		void Invalidate(Ref<ScriptClass> scriptClass, UUID entityID);

	private:
		void LoadMethods();
		void LoadFields();

	private:
		Ref<ScriptClass> m_EntityClass;
		Ref<ScriptClass> m_ScriptClass;

		GCHandle m_Handle = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		MonoMethod* m_OnDestroyMethod = nullptr;

		MonoMethod* m_OnCollisionEnter2DMethod = nullptr;
		MonoMethod* m_OnCollisionExit2DMethod = nullptr;
		MonoMethod* m_OnSensorEnter2DMethod = nullptr;
		MonoMethod* m_OnSensorExit2DMethod = nullptr;

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
		static MonoImage* GetAppAssemblyImage();

		static void OnRuntimeBegin();
		static void OnRuntimeEnd();

		static ScriptInstance* CreateInstance(Entity entity, const eastl::string& name);
		static bool HasInstance(Entity entity, const eastl::string& name);
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

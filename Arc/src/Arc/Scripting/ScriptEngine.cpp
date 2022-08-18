#include "arcpch.h"
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-debug.h>

#ifdef ARC_DEBUG
#include <mono/metadata/debug-helpers.h>
#endif // ARC_DEBUG

#include "GCManager.h"
#include "ScriptEngineRegistry.h"

namespace ArcEngine
{
	struct ScriptEngineData
	{
		MonoDomain* Domain;
		MonoAssembly* CoreAssembly;
		MonoAssembly* ClientAssembly;
		MonoImage* CoreImage;
		MonoImage* ClientImage;

		MonoClass* EntityClass;

		eastl::unordered_map<eastl::string, Ref<ScriptClass>> s_EntityClasses;
		eastl::unordered_map<UUID, eastl::unordered_map<eastl::string, Ref<ScriptInstance>>> s_EntityInstances;
		eastl::unordered_map<UUID, eastl::unordered_map<eastl::string, Ref<ScriptInstance>>> s_EntityRuntimeInstances;
	};

	static Ref<ScriptEngineData> s_Data;

	Scene* ScriptEngine::s_CurrentScene = nullptr;

	void ScriptEngine::Init(const char* coreAssemblyPath)
	{
		ARC_PROFILE_SCOPE();

		mono_set_dirs("C:/Program Files/Mono/lib",
        "C:/Program Files/Mono/etc");

		s_Data = CreateRef<ScriptEngineData>();

		s_Data->Domain = mono_jit_init("ScriptEngine");
		ARC_CORE_ASSERT(s_Data->Domain, "Could not initialize domain");

		mono_debug_domain_create(s_Data->Domain);
		mono_thread_set_main(mono_thread_current());

		GCManager::Init();

		LoadCoreAssembly(coreAssemblyPath);
	}

	void ScriptEngine::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		GCManager::Shutdown();

		s_Data->s_EntityClasses.clear();

		mono_jit_cleanup(s_Data->Domain);
	}

	void ScriptEngine::LoadCoreAssembly(const char* path)
	{
		ARC_PROFILE_SCOPE();

		s_Data->CoreAssembly = mono_domain_assembly_open(s_Data->Domain, path);
		if (!s_Data->CoreAssembly)
		{
			ARC_CORE_ERROR("Could not open assembly: {0}", path);
			return;
		}

		s_Data->CoreImage = mono_assembly_get_image(s_Data->CoreAssembly);
		if (!s_Data->CoreImage)
		{
			ARC_CORE_ERROR("Could not get image from assembly: {0}", path);
			return;
		}

		GCManager::CollectGarbage();
		ScriptEngineRegistry::RegisterAll();
	}

	void ScriptEngine::LoadClientAssembly(const char* path)
	{
		ARC_PROFILE_SCOPE();

		s_Data->ClientAssembly = mono_domain_assembly_open(s_Data->Domain, path);
		if (!s_Data->ClientAssembly)
		{
			ARC_CORE_ERROR("Could not open assembly: {0}", path);
			return;
		}

		s_Data->ClientImage = mono_assembly_get_image(s_Data->ClientAssembly);
		if (!s_Data->ClientImage)
		{
			ARC_CORE_ERROR("Could not get image from assembly: {0}", path);
			return;
		}

		GCManager::CollectGarbage();

		s_Data->s_EntityClasses.clear();
		s_Data->EntityClass = mono_class_from_name(s_Data->CoreImage, "ArcEngine", "Entity");
		LoadAssemblyClasses(s_Data->ClientAssembly);
	}

	void ArcEngine::ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		ARC_PROFILE_SCOPE();

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);

		for (int32_t i = 0; i < numTypes; ++i)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			eastl::string fullname = fmt::format("{}.{}", nameSpace, name).c_str();

			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);
			bool isEntity = mono_class_is_subclass_of(monoClass, s_Data->EntityClass, false);
			if (isEntity)
				s_Data->s_EntityClasses[fullname] = CreateRef<ScriptClass>(nameSpace, name);

			ARC_CORE_TRACE(fullname.c_str());
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreImage;
	}

	GCHandle ScriptEngine::CreateInstance(Entity entity, const eastl::string& name)
	{
		auto& scriptClass = s_Data->s_EntityClasses.at(name);
		Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(scriptClass, entity);
		s_Data->s_EntityInstances[entity.GetUUID()][name] = instance;
		return instance->GetHandle();
	}

	GCHandle ScriptEngine::CreateInstanceRuntime(Entity entity, const eastl::string& name)
	{
		UUID id = entity.GetUUID();
		auto& instance = s_Data->s_EntityInstances[id].at(name);
		Ref<ScriptInstance> copy = CreateRef<ScriptInstance>(instance, entity);
		s_Data->s_EntityRuntimeInstances[id][name] = copy;
		return copy->GetHandle();
	}

	Ref<ScriptInstance>& ScriptEngine::GetInstance(Entity entity, const eastl::string& name)
	{
		return s_Data->s_EntityInstances[entity.GetUUID()].at(name);
	}

	Ref<ScriptInstance>& ScriptEngine::GetInstanceRuntime(Entity entity, const eastl::string& name)
	{
		return s_Data->s_EntityRuntimeInstances[entity.GetUUID()].at(name);
	}

	MonoDomain* ScriptEngine::GetDomain()
	{
		return s_Data->Domain;
	}

	void ScriptEngine::ReleaseObjectReference(const GCHandle handle)
	{
		ARC_PROFILE_SCOPE();

		GCManager::ReleaseObjectReference(handle);
	}

	eastl::unordered_map<eastl::string, Field>& ScriptEngine::GetFields(const char* className)
	{
		return s_Data->s_EntityClasses.at(className)->GetFields();
	}

	eastl::unordered_map<eastl::string, Ref<ScriptClass>>& ScriptEngine::GetClasses()
	{
		return s_Data->s_EntityClasses;
	}

	void ScriptEngine::SetProperty(GCHandle handle, void* property, void** params)
	{
		ARC_PROFILE_SCOPE();

		MonoMethod* method = mono_property_get_set_method((MonoProperty*)property);
		MonoObject* reference = GCManager::GetReferencedObject(handle);
		if (reference)
			mono_runtime_invoke(method, reference, params, nullptr);
	}

	MonoProperty* ScriptEngine::GetProperty(const char* className, const char* propertyName)
	{
		ARC_PROFILE_SCOPE();

		return s_Data->s_EntityClasses.at(className)->GetProperty(className, propertyName);
	}


	ScriptClass::ScriptClass(MonoClass* monoClass)
		: m_MonoClass(monoClass)
	{
		LoadFields();
	}

	// ScriptClass
	ScriptClass::ScriptClass(const eastl::string& classNamespace, const eastl::string& className)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(s_Data->ClientImage, classNamespace.c_str(), className.c_str());
		LoadFields();
	}

	GCHandle ScriptClass::Instantiate()
	{
		MonoObject* object = mono_object_new(s_Data->Domain, m_MonoClass);
		if (object)
		{
			mono_runtime_object_init(object);
			return GCManager::CreateObjectReference(object, false);
		}

		return nullptr;
	}

	MonoMethod* ScriptClass::GetMethod(const eastl::string& signature)
	{
		eastl::string desc = m_ClassNamespace + "." + m_ClassName + ":" + signature;
		MonoMethodDesc* methodDesc = mono_method_desc_new(desc.c_str(), true);
		return mono_method_desc_search_in_class(methodDesc, m_MonoClass);
	}

	GCHandle ScriptClass::InvokeMethod(GCHandle gcHandle, MonoMethod* method, void** params)
	{
		MonoObject* reference = GCManager::GetReferencedObject(gcHandle);
		mono_runtime_invoke(method, reference, params, nullptr);
		return gcHandle;
	}

	eastl::unordered_map<eastl::string, Field>& ScriptClass::GetFields()
	{
		return m_Fields;
	}

	MonoProperty* ScriptClass::GetProperty(const char* className, const char* propertyName)
	{
		ARC_PROFILE_SCOPE();

		eastl::string key = eastl::string(className) + propertyName;
		if (m_Properties.find_as(key) != m_Properties.end())
			return m_Properties.at(key);

		MonoProperty* property = mono_class_get_property_from_name(m_MonoClass, propertyName);
		if (property)
			m_Properties.emplace(key, property);
		else
			ARC_CORE_ERROR("Property: {0} not found in class {1}", propertyName, className);

		return property;
	}

	void ScriptClass::SetProperty(GCHandle gcHandle, void* property, void** params)
	{
		ARC_PROFILE_SCOPE();

		MonoMethod* method = mono_property_get_set_method((MonoProperty*)property);
		InvokeMethod(gcHandle, method, params);
	}

	void ScriptClass::LoadFields()
	{
		m_Fields.clear();

		MonoClassField* iter;
		void* ptr = 0;
		while ((iter = mono_class_get_fields(m_MonoClass, &ptr)) != NULL)
		{
			const char* propertyName = mono_field_get_name(iter);
			uint32_t flags = mono_field_get_flags(iter);
			if ((flags & MONO_FIELD_ATTR_PUBLIC) == 0)
				continue;

			MonoType* fieldType = mono_field_get_type(iter);
			Field::FieldType type = Field::GetFieldType(fieldType);
			MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(m_MonoClass, iter);

			m_Fields.emplace(propertyName, Field(propertyName, type, iter));
		}
	}



	// ScriptInstance
	ScriptInstance::ScriptInstance(Ref<ScriptInstance> scriptInstance, Entity entity)
	{
		m_ScriptClass = scriptInstance->m_ScriptClass;

		MonoObject* copy = mono_object_clone(GCManager::GetReferencedObject(scriptInstance->m_Handle));
		m_Handle = GCManager::CreateObjectReference(copy, false);

		ScriptClass entityClass = ScriptClass(s_Data->EntityClass);
		m_Constructor = entityClass.GetMethod(".ctor(ulong)");
		void* params = &entity.GetUUID();
		entityClass.InvokeMethod(m_Handle, m_Constructor, &params);

		m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate()");
		m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate(single)");
		m_OnDestroyMethod = m_ScriptClass->GetMethod("OnDestroy()");
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Handle = scriptClass->Instantiate();

		ScriptClass entityClass = ScriptClass(s_Data->EntityClass);
		m_Constructor = entityClass.GetMethod(".ctor(ulong)");
		void* params = &entity.GetUUID();
		entityClass.InvokeMethod(m_Handle, m_Constructor, &params);

		m_OnCreateMethod = scriptClass->GetMethod("OnCreate()");
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate(single)");
		m_OnDestroyMethod = scriptClass->GetMethod("OnDestroy()");
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Handle, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (m_OnUpdateMethod)
		{
			void* params = &ts;
			m_ScriptClass->InvokeMethod(m_Handle, m_OnUpdateMethod, &params);
		}
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (m_OnDestroyMethod)
			m_ScriptClass->InvokeMethod(m_Handle, m_OnDestroyMethod);
	}
}
